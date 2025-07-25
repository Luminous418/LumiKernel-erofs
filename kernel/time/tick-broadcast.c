/*
 * linux/kernel/time/tick-broadcast.c
 *
 * This file contains functions which emulate a local clock-event
 * device via a broadcast event source.
 *
 * Copyright(C) 2005-2006, Thomas Gleixner <tglx@linutronix.de>
 * Copyright(C) 2005-2007, Red Hat, Inc., Ingo Molnar
 * Copyright(C) 2006-2007, Timesys Corp., Thomas Gleixner
 *
 * This code is licenced under the GPL version 2. For details see
 * kernel-base/COPYING.
 */
#include <linux/cpu.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/profile.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/module.h>
#include <linux/sched/clock.h>

#include "tick-internal.h"

#define CONFIG_MTK_TICK_BROADCAST_AEE_DUMP

#if defined(CONFIG_MTK_RAM_CONSOLE)
#if defined(CONFIG_MTK_TICK_BROADCAST_AEE_DUMP)
#include <linux/cpumask.h>
#include <mt-plat/mtk_ram_console.h>

#define _MTK_TICK_BROADCAST_AEE_DUMP
#endif
#endif

/*
 * Broadcast support for broken x86 hardware, where the local apic
 * timer stops in C3 state.
 */

static struct tick_device tick_broadcast_device;
static cpumask_var_t tick_broadcast_mask __cpumask_var_read_mostly;
static cpumask_var_t tick_broadcast_on __cpumask_var_read_mostly;
static cpumask_var_t tmpmask __cpumask_var_read_mostly;
static int tick_broadcast_forced;

static __cacheline_aligned_in_smp DEFINE_RAW_SPINLOCK(tick_broadcast_lock);

#ifdef CONFIG_TICK_ONESHOT
static void tick_broadcast_setup_oneshot(struct clock_event_device *bc);
static void tick_broadcast_clear_oneshot(int cpu);
static void tick_resume_broadcast_oneshot(struct clock_event_device *bc);
#else
static inline void tick_broadcast_setup_oneshot(struct clock_event_device *bc) { BUG(); }
static inline void tick_broadcast_clear_oneshot(int cpu) { }
static inline void tick_resume_broadcast_oneshot(struct clock_event_device *bc) { }
#endif

/*
 * Debugging: see timer_list.c
 */
struct tick_device *tick_get_broadcast_device(void)
{
	return &tick_broadcast_device;
}

struct cpumask *tick_get_broadcast_mask(void)
{
	return tick_broadcast_mask;
}

/*
 * Start the device in periodic mode
 */
static void tick_broadcast_start_periodic(struct clock_event_device *bc)
{
	if (bc)
		tick_setup_periodic(bc, 1);
}

/*
 * Check, if the device can be utilized as broadcast device:
 */
static bool tick_check_broadcast_device(struct clock_event_device *curdev,
					struct clock_event_device *newdev)
{
	if ((newdev->features & CLOCK_EVT_FEAT_DUMMY) ||
	    (newdev->features & CLOCK_EVT_FEAT_PERCPU) ||
	    (newdev->features & CLOCK_EVT_FEAT_C3STOP))
		return false;

	if (tick_broadcast_device.mode == TICKDEV_MODE_ONESHOT &&
	    !(newdev->features & CLOCK_EVT_FEAT_ONESHOT))
		return false;

	return !curdev || newdev->rating > curdev->rating;
}

/*
 * Conditionally install/replace broadcast device
 */
void tick_install_broadcast_device(struct clock_event_device *dev)
{
	struct clock_event_device *cur = tick_broadcast_device.evtdev;

	if (!tick_check_broadcast_device(cur, dev))
		return;

	if (!try_module_get(dev->owner))
		return;

	clockevents_exchange_device(cur, dev);
	if (cur)
		cur->event_handler = clockevents_handle_noop;
	tick_broadcast_device.evtdev = dev;
	if (!cpumask_empty(tick_broadcast_mask))
		tick_broadcast_start_periodic(dev);
	/*
	 * Inform all cpus about this. We might be in a situation
	 * where we did not switch to oneshot mode because the per cpu
	 * devices are affected by CLOCK_EVT_FEAT_C3STOP and the lack
	 * of a oneshot capable broadcast device. Without that
	 * notification the systems stays stuck in periodic mode
	 * forever.
	 */
	if (dev->features & CLOCK_EVT_FEAT_ONESHOT)
		tick_clock_notify();
}

/*
 * Check, if the device is the broadcast device
 */
int tick_is_broadcast_device(struct clock_event_device *dev)
{
	return (dev && tick_broadcast_device.evtdev == dev);
}

int tick_broadcast_update_freq(struct clock_event_device *dev, u32 freq)
{
	int ret = -ENODEV;

	if (tick_is_broadcast_device(dev)) {
		raw_spin_lock(&tick_broadcast_lock);
		ret = __clockevents_update_freq(dev, freq);
		raw_spin_unlock(&tick_broadcast_lock);
	}
	return ret;
}


static void err_broadcast(const struct cpumask *mask)
{
	pr_crit_once("Failed to broadcast timer tick. Some CPUs may be unresponsive.\n");
}

static void tick_device_setup_broadcast_func(struct clock_event_device *dev)
{
	if (!dev->broadcast)
		dev->broadcast = tick_broadcast;
	if (!dev->broadcast) {
		pr_warn_once("%s depends on broadcast, but no broadcast function available\n",
			     dev->name);
		dev->broadcast = err_broadcast;
	}
}

/*
 * Check, if the device is disfunctional and a place holder, which
 * needs to be handled by the broadcast device.
 */
int tick_device_uses_broadcast(struct clock_event_device *dev, int cpu)
{
	struct clock_event_device *bc = tick_broadcast_device.evtdev;
	unsigned long flags;
	int ret = 0;

	raw_spin_lock_irqsave(&tick_broadcast_lock, flags);

	/*
	 * Devices might be registered with both periodic and oneshot
	 * mode disabled. This signals, that the device needs to be
	 * operated from the broadcast device and is a placeholder for
	 * the cpu local device.
	 */
	if (!tick_device_is_functional(dev)) {
		dev->event_handler = tick_handle_periodic;
		tick_device_setup_broadcast_func(dev);
		cpumask_set_cpu(cpu, tick_broadcast_mask);
		if (tick_broadcast_device.mode == TICKDEV_MODE_PERIODIC)
			tick_broadcast_start_periodic(bc);
		else
			tick_broadcast_setup_oneshot(bc);
		ret = 1;
	} else {
		/*
		 * Clear the broadcast bit for this cpu if the
		 * device is not power state affected.
		 */
		if (!(dev->features & CLOCK_EVT_FEAT_C3STOP))
			cpumask_clear_cpu(cpu, tick_broadcast_mask);
		else
			tick_device_setup_broadcast_func(dev);

		/*
		 * Clear the broadcast bit if the CPU is not in
		 * periodic broadcast on state.
		 */
		if (!cpumask_test_cpu(cpu, tick_broadcast_on))
			cpumask_clear_cpu(cpu, tick_broadcast_mask);

		switch (tick_broadcast_device.mode) {
		case TICKDEV_MODE_ONESHOT:
			/*
			 * If the system is in oneshot mode we can
			 * unconditionally clear the oneshot mask bit,
			 * because the CPU is running and therefore
			 * not in an idle state which causes the power
			 * state affected device to stop. Let the
			 * caller initialize the device.
			 */
			tick_broadcast_clear_oneshot(cpu);
			ret = 0;
			break;

		case TICKDEV_MODE_PERIODIC:
			/*
			 * If the system is in periodic mode, check
			 * whether the broadcast device can be
			 * switched off now.
			 */
			if (cpumask_empty(tick_broadcast_mask) && bc)
				clockevents_shutdown(bc);
			/*
			 * If we kept the cpu in the broadcast mask,
			 * tell the caller to leave the per cpu device
			 * in shutdown state. The periodic interrupt
			 * is delivered by the broadcast device, if
			 * the broadcast device exists and is not
			 * hrtimer based.
			 */
			if (bc && !(bc->features & CLOCK_EVT_FEAT_HRTIMER))
				ret = cpumask_test_cpu(cpu, tick_broadcast_mask);
			break;
		default:
			break;
		}
	}
	raw_spin_unlock_irqrestore(&tick_broadcast_lock, flags);
	return ret;
}

#ifdef CONFIG_GENERIC_CLOCKEVENTS_BROADCAST
int tick_receive_broadcast(void)
{
	struct tick_device *td = this_cpu_ptr(&tick_cpu_device);
	struct clock_event_device *evt = td->evtdev;

	if (!evt)
		return -ENODEV;

	if (!evt->event_handler)
		return -EINVAL;

	evt->event_handler(evt);
	return 0;
}
#endif

/*
 * Broadcast the event to the cpus, which are set in the mask (mangled).
 */
static bool tick_do_broadcast(struct cpumask *mask)
{
	int cpu = smp_processor_id();
	struct tick_device *td;
	bool local = false;

	/*
	 * Check, if the current cpu is in the mask
	 */
	if (cpumask_test_cpu(cpu, mask)) {
		struct clock_event_device *bc = tick_broadcast_device.evtdev;

		cpumask_clear_cpu(cpu, mask);
		/*
		 * We only run the local handler, if the broadcast
		 * device is not hrtimer based. Otherwise we run into
		 * a hrtimer recursion.
		 *
		 * local timer_interrupt()
		 *   local_handler()
		 *     expire_hrtimers()
		 *       bc_handler()
		 *         local_handler()
		 *	     expire_hrtimers()
		 */
		local = !(bc->features & CLOCK_EVT_FEAT_HRTIMER);
	}

	if (!cpumask_empty(mask)) {
		/*
		 * It might be necessary to actually check whether the devices
		 * have different broadcast functions. For now, just use the
		 * one of the first device. This works as long as we have this
		 * misfeature only on x86 (lapic)
		 */
		td = &per_cpu(tick_cpu_device, cpumask_first(mask));
		td->evtdev->broadcast(mask);
	}
	return local;
}

/*
 * Periodic broadcast:
 * - invoke the broadcast handlers
 */
static bool tick_do_periodic_broadcast(void)
{
	cpumask_and(tmpmask, cpu_online_mask, tick_broadcast_mask);
	return tick_do_broadcast(tmpmask);
}

/*
 * Event handler for periodic broadcast ticks
 */
static void tick_handle_periodic_broadcast(struct clock_event_device *dev)
{
	struct tick_device *td = this_cpu_ptr(&tick_cpu_device);
	bool bc_local;

	raw_spin_lock(&tick_broadcast_lock);

	/* Handle spurious interrupts gracefully */
	if (clockevent_state_shutdown(tick_broadcast_device.evtdev)) {
		raw_spin_unlock(&tick_broadcast_lock);
		return;
	}

	bc_local = tick_do_periodic_broadcast();

	if (clockevent_state_oneshot(dev)) {
		ktime_t next = ktime_add(dev->next_event, tick_period);

		clockevents_program_event(dev, next, true);
	}
	raw_spin_unlock(&tick_broadcast_lock);

	/*
	 * We run the handler of the local cpu after dropping
	 * tick_broadcast_lock because the handler might deadlock when
	 * trying to switch to oneshot mode.
	 */
	if (bc_local)
		td->evtdev->event_handler(td->evtdev);
}

/**
 * tick_broadcast_control - Enable/disable or force broadcast mode
 * @mode:	The selected broadcast mode
 *
 * Called when the system enters a state where affected tick devices
 * might stop. Note: TICK_BROADCAST_FORCE cannot be undone.
 */
void tick_broadcast_control(enum tick_broadcast_mode mode)
{
	struct clock_event_device *bc, *dev;
	struct tick_device *td;
	int cpu, bc_stopped;
	unsigned long flags;

	/* Protects also the local clockevent device. */
	raw_spin_lock_irqsave(&tick_broadcast_lock, flags);
	td = this_cpu_ptr(&tick_cpu_device);
	dev = td->evtdev;

	/*
	 * Is the device not affected by the powerstate ?
	 */
	if (!dev || !(dev->features & CLOCK_EVT_FEAT_C3STOP))
		goto out;

	if (!tick_device_is_functional(dev))
		goto out;

	cpu = smp_processor_id();
	bc = tick_broadcast_device.evtdev;
	bc_stopped = cpumask_empty(tick_broadcast_mask);

	switch (mode) {
	case TICK_BROADCAST_FORCE:
		tick_broadcast_forced = 1;
	case TICK_BROADCAST_ON:
		cpumask_set_cpu(cpu, tick_broadcast_on);
		if (!cpumask_test_and_set_cpu(cpu, tick_broadcast_mask)) {
			/*
			 * Only shutdown the cpu local device, if:
			 *
			 * - the broadcast device exists
			 * - the broadcast device is not a hrtimer based one
			 * - the broadcast device is in periodic mode to
			 *   avoid a hickup during switch to oneshot mode
			 */
			if (bc && !(bc->features & CLOCK_EVT_FEAT_HRTIMER) &&
			    tick_broadcast_device.mode == TICKDEV_MODE_PERIODIC)
				clockevents_shutdown(dev);
		}
		break;

	case TICK_BROADCAST_OFF:
		if (tick_broadcast_forced)
			break;
		cpumask_clear_cpu(cpu, tick_broadcast_on);
		if (!tick_device_is_functional(dev))
			break;
		if (cpumask_test_and_clear_cpu(cpu, tick_broadcast_mask)) {
			if (tick_broadcast_device.mode ==
			    TICKDEV_MODE_PERIODIC)
				tick_setup_periodic(dev, 0);
		}
		break;
	}

	if (bc) {
		if (cpumask_empty(tick_broadcast_mask)) {
			if (!bc_stopped)
				clockevents_shutdown(bc);
		} else if (bc_stopped) {
			if (tick_broadcast_device.mode == TICKDEV_MODE_PERIODIC)
				tick_broadcast_start_periodic(bc);
			else
				tick_broadcast_setup_oneshot(bc);
		}
	}
out:
	raw_spin_unlock_irqrestore(&tick_broadcast_lock, flags);
}
EXPORT_SYMBOL_GPL(tick_broadcast_control);

/*
 * Set the periodic handler depending on broadcast on/off
 */
void tick_set_periodic_handler(struct clock_event_device *dev, int broadcast)
{
	if (!broadcast)
		dev->event_handler = tick_handle_periodic;
	else
		dev->event_handler = tick_handle_periodic_broadcast;
}

#ifdef CONFIG_HOTPLUG_CPU
/*
 * Remove a CPU from broadcasting
 */
void tick_shutdown_broadcast(unsigned int cpu)
{
	struct clock_event_device *bc;
	unsigned long flags;

	raw_spin_lock_irqsave(&tick_broadcast_lock, flags);

	bc = tick_broadcast_device.evtdev;
	cpumask_clear_cpu(cpu, tick_broadcast_mask);
	cpumask_clear_cpu(cpu, tick_broadcast_on);

	if (tick_broadcast_device.mode == TICKDEV_MODE_PERIODIC) {
		if (bc && cpumask_empty(tick_broadcast_mask))
			clockevents_shutdown(bc);
	}

	raw_spin_unlock_irqrestore(&tick_broadcast_lock, flags);
}
#endif

void tick_suspend_broadcast(void)
{
	struct clock_event_device *bc;
	unsigned long flags;

	raw_spin_lock_irqsave(&tick_broadcast_lock, flags);

	bc = tick_broadcast_device.evtdev;
	if (bc)
		clockevents_shutdown(bc);

	raw_spin_unlock_irqrestore(&tick_broadcast_lock, flags);
}

/*
 * This is called from tick_resume_local() on a resuming CPU. That's
 * called from the core resume function, tick_unfreeze() and the magic XEN
 * resume hackery.
 *
 * In none of these cases the broadcast device mode can change and the
 * bit of the resuming CPU in the broadcast mask is safe as well.
 */
bool tick_resume_check_broadcast(void)
{
	if (tick_broadcast_device.mode == TICKDEV_MODE_ONESHOT)
		return false;
	else
		return cpumask_test_cpu(smp_processor_id(), tick_broadcast_mask);
}

void tick_resume_broadcast(void)
{
	struct clock_event_device *bc;
	unsigned long flags;

	raw_spin_lock_irqsave(&tick_broadcast_lock, flags);

	bc = tick_broadcast_device.evtdev;

	if (bc) {
		clockevents_tick_resume(bc);

		switch (tick_broadcast_device.mode) {
		case TICKDEV_MODE_PERIODIC:
			if (!cpumask_empty(tick_broadcast_mask))
				tick_broadcast_start_periodic(bc);
			break;
		case TICKDEV_MODE_ONESHOT:
			if (!cpumask_empty(tick_broadcast_mask))
				tick_resume_broadcast_oneshot(bc);
			break;
		}
	}
	raw_spin_unlock_irqrestore(&tick_broadcast_lock, flags);
}

#ifdef CONFIG_TICK_ONESHOT

static cpumask_var_t tick_broadcast_oneshot_mask __cpumask_var_read_mostly;
static cpumask_var_t tick_broadcast_pending_mask __cpumask_var_read_mostly;
static cpumask_var_t tick_broadcast_force_mask __cpumask_var_read_mostly;

#ifdef _MTK_TICK_BROADCAST_AEE_DUMP

struct tick_broadcast_history_struct {
	unsigned long long time_enter;
	unsigned long long time_exit;
	int ret_enter;
};

static struct tick_broadcast_history_struct tick_broadcast_history[NR_CPUS];
static char tick_broadcast_mtk_aee_dump_buf[128];
static uint64_t tick_broadcast_enter_prev;
static uint64_t tick_broadcast_enter_count[NR_CPUS];
static uint64_t tick_broadcast_fail_count[NR_CPUS];
static uint64_t tick_broadcast_success_count[NR_CPUS];
static uint64_t tick_broadcast_interrupt_count[NR_CPUS];
static uint64_t _tick_broadcast_enter_count[NR_CPUS];
static uint64_t _tick_broadcast_fail_count[NR_CPUS];
static uint64_t _tick_broadcast_success_count[NR_CPUS];
static uint64_t _tick_broadcast_interrupt_count[NR_CPUS];
static struct cpumask _tb_dbg_oneshot_mask;
static struct cpumask _tb_dbg_pending_mask;
static struct cpumask _tb_dbg_force_mask;

#define LOG_BUF_LEN         1024
struct tick_broadcast_dump_buf {
	char buf[LOG_BUF_LEN];
	char *p_idx;
};

#define reset_bc_dump_buf(buffer) ((buffer).p_idx = (buffer).buf)
#define get_bc_dump_buf(buffer)   ((buffer).buf)
#define bc_dump_buf_append(buffer, fmt, args...)           \
	((buffer).p_idx += snprintf((buffer).p_idx,           \
					LOG_BUF_LEN - strlen((buffer).buf), \
					fmt, ##args))
struct tick_broadcast_dump_buf bc_dump_buf;

void tick_broadcast_mtk_aee_dump(void)
{
	int i, ret = -1;

	/*
	 * Notice: printk cannot be used during AEE flow to avoid lock issues.
	 */

	/* tick_broadcast_oneshot_mask */

	memset(tick_broadcast_mtk_aee_dump_buf, 0,
		sizeof(tick_broadcast_mtk_aee_dump_buf));
	ret = snprintf(tick_broadcast_mtk_aee_dump_buf,
		sizeof(tick_broadcast_mtk_aee_dump_buf),
		"[TICK] oneshot_mask: %*pbl\n",
		cpumask_pr_args(tick_broadcast_oneshot_mask));
	if (ret >= 0)
		aee_sram_fiq_log(tick_broadcast_mtk_aee_dump_buf);

	/* tick_broadcast_pending_mask */

	memset(tick_broadcast_mtk_aee_dump_buf, 0,
		sizeof(tick_broadcast_mtk_aee_dump_buf));
	ret = snprintf(tick_broadcast_mtk_aee_dump_buf,
		sizeof(tick_broadcast_mtk_aee_dump_buf),
		"[TICK] pending_mask: %*pbl\n",
		cpumask_pr_args(tick_broadcast_pending_mask));
	if (ret >= 0)
		aee_sram_fiq_log(tick_broadcast_mtk_aee_dump_buf);

	/* tick_broadcast_force_mask */

	memset(tick_broadcast_mtk_aee_dump_buf, 0,
		sizeof(tick_broadcast_mtk_aee_dump_buf));
	ret = snprintf(tick_broadcast_mtk_aee_dump_buf,
		sizeof(tick_broadcast_mtk_aee_dump_buf),
		"[TICK] force_mask: %*pbl\n",
		cpumask_pr_args(tick_broadcast_force_mask));
	if (ret >= 0)
		aee_sram_fiq_log(tick_broadcast_mtk_aee_dump_buf);

	for_each_possible_cpu(i) {
		/* to avoid unexpected overrun */
		if (i >= num_possible_cpus())
			break;
		memset(tick_broadcast_mtk_aee_dump_buf, 0,
			sizeof(tick_broadcast_mtk_aee_dump_buf));
		ret = snprintf(tick_broadcast_mtk_aee_dump_buf,
			sizeof(tick_broadcast_mtk_aee_dump_buf),
			"[TICK] cpu %d, %llu, %d, %llu\n",
			i, tick_broadcast_history[i].time_enter,
			tick_broadcast_history[i].ret_enter,
			tick_broadcast_history[i].time_exit);
		if (ret >= 0)
			aee_sram_fiq_log(tick_broadcast_mtk_aee_dump_buf);
	}
}
#endif

/*
 * Exposed for debugging: see timer_list.c
 */
struct cpumask *tick_get_broadcast_oneshot_mask(void)
{
	return tick_broadcast_oneshot_mask;
}

/*
 * Called before going idle with interrupts disabled. Checks whether a
 * broadcast event from the other core is about to happen. We detected
 * that in tick_broadcast_oneshot_control(). The callsite can use this
 * to avoid a deep idle transition as we are about to get the
 * broadcast IPI right away.
 */
int tick_check_broadcast_expired(void)
{
	return cpumask_test_cpu(smp_processor_id(), tick_broadcast_force_mask);
}

/*
 * Set broadcast interrupt affinity
 */
static void tick_broadcast_set_affinity(struct clock_event_device *bc,
					const struct cpumask *cpumask)
{
	if (!(bc->features & CLOCK_EVT_FEAT_DYNIRQ))
		return;

	if (cpumask_equal(bc->cpumask, cpumask))
		return;

	bc->cpumask = cpumask;
	irq_set_affinity(bc->irq, bc->cpumask);

	/* MTK PATCH: record new target cpu for dynamic irq affinity */
	bc->irq_affinity_on = cpumask_first(cpumask);
}

static void tick_broadcast_set_event(struct clock_event_device *bc, int cpu,
				     ktime_t expires)
{
	if (!clockevent_state_oneshot(bc))
		clockevents_switch_state(bc, CLOCK_EVT_STATE_ONESHOT);

	clockevents_program_event(bc, expires, 1);
	tick_broadcast_set_affinity(bc, cpumask_of(cpu));
}

static void tick_resume_broadcast_oneshot(struct clock_event_device *bc)
{
	clockevents_switch_state(bc, CLOCK_EVT_STATE_ONESHOT);
}

/*
 * Called from irq_enter() when idle was interrupted to reenable the
 * per cpu device.
 */
void tick_check_oneshot_broadcast_this_cpu(void)
{
	if (cpumask_test_cpu(smp_processor_id(), tick_broadcast_oneshot_mask)) {
		struct tick_device *td = this_cpu_ptr(&tick_cpu_device);

		/*
		 * We might be in the middle of switching over from
		 * periodic to oneshot. If the CPU has not yet
		 * switched over, leave the device alone.
		 */
		if (td->mode == TICKDEV_MODE_ONESHOT) {
			clockevents_switch_state(td->evtdev,
					      CLOCK_EVT_STATE_ONESHOT);
		}
	}
}

/*
 * Handle oneshot mode broadcasting
 */
static void tick_handle_oneshot_broadcast(struct clock_event_device *dev)
{
	struct tick_device *td;
	ktime_t now, next_event;
	int cpu, next_cpu = 0;
	bool bc_local;

	raw_spin_lock(&tick_broadcast_lock);
#ifdef _MTK_TICK_BROADCAST_AEE_DUMP
	tick_broadcast_interrupt_count[smp_processor_id()]++;
#endif
	dev->next_event = KTIME_MAX;
	next_event = KTIME_MAX;
	cpumask_clear(tmpmask);
	now = ktime_get();
	/* Find all expired events */
	for_each_cpu(cpu, tick_broadcast_oneshot_mask) {
		/*
		 * Required for !SMP because for_each_cpu() reports
		 * unconditionally CPU0 as set on UP kernels.
		 */
		if (!IS_ENABLED(CONFIG_SMP) &&
		    cpumask_empty(tick_broadcast_oneshot_mask))
			break;

		td = &per_cpu(tick_cpu_device, cpu);
		if (td->evtdev->next_event <= now) {
			cpumask_set_cpu(cpu, tmpmask);
			/*
			 * Mark the remote cpu in the pending mask, so
			 * it can avoid reprogramming the cpu local
			 * timer in tick_broadcast_oneshot_control().
			 */
			cpumask_set_cpu(cpu, tick_broadcast_pending_mask);
		} else if (td->evtdev->next_event < next_event) {
			next_event = td->evtdev->next_event;
			next_cpu = cpu;
		}
	}

	/*
	 * Remove the current cpu from the pending mask. The event is
	 * delivered immediately in tick_do_broadcast() !
	 */
	cpumask_clear_cpu(smp_processor_id(), tick_broadcast_pending_mask);

	/* Take care of enforced broadcast requests */
	cpumask_or(tmpmask, tmpmask, tick_broadcast_force_mask);
	cpumask_clear(tick_broadcast_force_mask);

	/*
	 * Sanity check. Catch the case where we try to broadcast to
	 * offline cpus.
	 */
	if (WARN_ON_ONCE(!cpumask_subset(tmpmask, cpu_online_mask)))
		cpumask_and(tmpmask, tmpmask, cpu_online_mask);

	/*
	 * Wakeup the cpus which have an expired event.
	 */
	bc_local = tick_do_broadcast(tmpmask);

	/*
	 * Two reasons for reprogram:
	 *
	 * - The global event did not expire any CPU local
	 * events. This happens in dyntick mode, as the maximum PIT
	 * delta is quite small.
	 *
	 * - There are pending events on sleeping CPUs which were not
	 * in the event mask
	 */
	if (next_event != KTIME_MAX)
		tick_broadcast_set_event(dev, next_cpu, next_event);

	raw_spin_unlock(&tick_broadcast_lock);

	if (bc_local) {
		td = this_cpu_ptr(&tick_cpu_device);
		td->evtdev->event_handler(td->evtdev);
	}
}

static int broadcast_needs_cpu(struct clock_event_device *bc, int cpu)
{
	if (!(bc->features & CLOCK_EVT_FEAT_HRTIMER))
		return 0;
	if (bc->next_event == KTIME_MAX)
		return 0;
	return bc->bound_on == cpu ? -EBUSY : 0;
}

/*
 * MTK PATCH:
 *
 * For soc timer based broadcasting and irq affinity feature is enabled,
 * if the dying cpu is exactly the target of irq by soc timer, we shall
 * change the irq affinity to another online cpu to make sure irq will be
 * serviced.
 *
 * Returns:
 *  0:      Changing irq affinity target is not required.
 *  Others: Changing irq affinity target is required.
 */
static int broadcast_needs_cpu_soctimer(struct clock_event_device *bc, int cpu)
{
	/* for soc timer based broadcasting only (not hrtimer based) */
	if (bc->features & CLOCK_EVT_FEAT_HRTIMER)
		return 0;

	/* for dynamic irq affinity feature enabled only */
	if (!(bc->features & CLOCK_EVT_FEAT_DYNIRQ))
		return 0;

	return bc->irq_affinity_on == cpu ? -EBUSY : 0;
}

static void broadcast_shutdown_local(struct clock_event_device *bc,
				     struct clock_event_device *dev)
{
	/*
	 * For hrtimer based broadcasting we cannot shutdown the cpu
	 * local device if our own event is the first one to expire or
	 * if we own the broadcast timer.
	 */
	if (bc->features & CLOCK_EVT_FEAT_HRTIMER) {
		if (broadcast_needs_cpu(bc, smp_processor_id()))
			return;
		if (dev->next_event < bc->next_event)
			return;
	}
	clockevents_switch_state(dev, CLOCK_EVT_STATE_SHUTDOWN);
}

int __tick_broadcast_oneshot_control(enum tick_broadcast_state state)
{
	struct clock_event_device *bc, *dev;
	int cpu, ret = 0;
	ktime_t now;
#ifdef _MTK_TICK_BROADCAST_AEE_DUMP
	uint64_t enter_offset = 0;
	int i = 0, need_dump = 0, set_event = 0;
	unsigned long long now_sched_clock = sched_clock();
#endif

	/*
	 * If there is no broadcast device, tell the caller not to go
	 * into deep idle.
	 */
	if (!tick_broadcast_device.evtdev)
		return -EBUSY;

	dev = this_cpu_ptr(&tick_cpu_device)->evtdev;

	raw_spin_lock(&tick_broadcast_lock);
	bc = tick_broadcast_device.evtdev;
	cpu = smp_processor_id();

	if (state == TICK_BROADCAST_ENTER) {
		/*
		 * If the current CPU owns the hrtimer broadcast
		 * mechanism, it cannot go deep idle and we do not add
		 * the CPU to the broadcast mask. We don't have to go
		 * through the EXIT path as the local timer is not
		 * shutdown.
		 */
		ret = broadcast_needs_cpu(bc, cpu);
		if (ret)
			goto out;

		/*
		 * If the broadcast device is in periodic mode, we
		 * return.
		 */
		if (tick_broadcast_device.mode == TICKDEV_MODE_PERIODIC) {
			/* If it is a hrtimer based broadcast, return busy */
			if (bc->features & CLOCK_EVT_FEAT_HRTIMER)
				ret = -EBUSY;
			goto out;
		}

		if (!cpumask_test_and_set_cpu(cpu, tick_broadcast_oneshot_mask)) {
			WARN_ON_ONCE(cpumask_test_cpu(cpu, tick_broadcast_pending_mask));

			/* Conditionally shut down the local timer. */
			broadcast_shutdown_local(bc, dev);

			/*
			 * We only reprogram the broadcast timer if we
			 * did not mark ourself in the force mask and
			 * if the cpu local event is earlier than the
			 * broadcast event. If the current CPU is in
			 * the force mask, then we are going to be
			 * woken by the IPI right away; we return
			 * busy, so the CPU does not try to go deep
			 * idle.
			 */
			if (cpumask_test_cpu(cpu, tick_broadcast_force_mask)) {
				ret = -EBUSY;
			} else if (dev->next_event < bc->next_event) {
#ifdef _MTK_TICK_BROADCAST_AEE_DUMP
				set_event = true;
#endif
				tick_broadcast_set_event(bc, cpu, dev->next_event);
				/*
				 * In case of hrtimer broadcasts the
				 * programming might have moved the
				 * timer to this cpu. If yes, remove
				 * us from the broadcast mask and
				 * return busy.
				 */
				ret = broadcast_needs_cpu(bc, cpu);
				if (ret) {
					cpumask_clear_cpu(cpu,
						tick_broadcast_oneshot_mask);
				}
			}
		}
	} else {
		if (cpumask_test_and_clear_cpu(cpu, tick_broadcast_oneshot_mask)) {
			clockevents_switch_state(dev, CLOCK_EVT_STATE_ONESHOT);
			/*
			 * The cpu which was handling the broadcast
			 * timer marked this cpu in the broadcast
			 * pending mask and fired the broadcast
			 * IPI. So we are going to handle the expired
			 * event anyway via the broadcast IPI
			 * handler. No need to reprogram the timer
			 * with an already expired event.
			 */
			if (cpumask_test_and_clear_cpu(cpu,
				       tick_broadcast_pending_mask))
				goto out;

			/*
			 * Bail out if there is no next event.
			 */
			if (dev->next_event == KTIME_MAX)
				goto out;
			/*
			 * If the pending bit is not set, then we are
			 * either the CPU handling the broadcast
			 * interrupt or we got woken by something else.
			 *
			 * We are not longer in the broadcast mask, so
			 * if the cpu local expiry time is already
			 * reached, we would reprogram the cpu local
			 * timer with an already expired event.
			 *
			 * This can lead to a ping-pong when we return
			 * to idle and therefor rearm the broadcast
			 * timer before the cpu local timer was able
			 * to fire. This happens because the forced
			 * reprogramming makes sure that the event
			 * will happen in the future and depending on
			 * the min_delta setting this might be far
			 * enough out that the ping-pong starts.
			 *
			 * If the cpu local next_event has expired
			 * then we know that the broadcast timer
			 * next_event has expired as well and
			 * broadcast is about to be handled. So we
			 * avoid reprogramming and enforce that the
			 * broadcast handler, which did not run yet,
			 * will invoke the cpu local handler.
			 *
			 * We cannot call the handler directly from
			 * here, because we might be in a NOHZ phase
			 * and we did not go through the irq_enter()
			 * nohz fixups.
			 */
			now = ktime_get();
			if (dev->next_event <= now) {
				cpumask_set_cpu(cpu, tick_broadcast_force_mask);
				goto out;
			}
			/*
			 * We got woken by something else. Reprogram
			 * the cpu local timer device.
			 */
			tick_program_event(dev->next_event, 1);
		}
	}
out:
#ifdef _MTK_TICK_BROADCAST_AEE_DUMP
	if (state == TICK_BROADCAST_ENTER) {
		tick_broadcast_history[cpu].time_enter = now_sched_clock;
		tick_broadcast_history[cpu].ret_enter = ret;

		tick_broadcast_enter_count[cpu]++;
		if (set_event)
			tick_broadcast_success_count[cpu]++;
		if (ret < 0)
			tick_broadcast_fail_count[cpu]++;
	} else {
		tick_broadcast_history[cpu].time_exit = now_sched_clock;

		now_sched_clock = sched_clock();
		do_div(now_sched_clock, 1000000ULL);
		enter_offset = now_sched_clock -
			tick_broadcast_enter_prev;
		if (enter_offset > 5000) {
			tick_broadcast_enter_prev =
				now_sched_clock;
			need_dump = true;

			for_each_possible_cpu(i) {
				/* to avoid unexpected overrun */
				if (i >= num_possible_cpus())
					break;
				_tick_broadcast_enter_count[i] =
					tick_broadcast_enter_count[i];
				_tick_broadcast_fail_count[i] =
					tick_broadcast_fail_count[i];
				_tick_broadcast_success_count[i] =
					tick_broadcast_success_count[i];
				_tick_broadcast_interrupt_count[i] =
					tick_broadcast_interrupt_count[i];

				tick_broadcast_enter_count[i] = 0;
				tick_broadcast_fail_count[i] = 0;
				tick_broadcast_success_count[i] = 0;
				tick_broadcast_interrupt_count[i] = 0;

				cpumask_copy(&_tb_dbg_oneshot_mask,
					tick_broadcast_oneshot_mask);
				cpumask_copy(&_tb_dbg_pending_mask,
					tick_broadcast_pending_mask);
				cpumask_copy(&_tb_dbg_force_mask,
					tick_broadcast_force_mask);
			}
		}
	}
#endif
	raw_spin_unlock(&tick_broadcast_lock);
#ifdef _MTK_TICK_BROADCAST_AEE_DUMP
	if (need_dump) {
		reset_bc_dump_buf(bc_dump_buf);

		bc_dump_buf_append(bc_dump_buf,
			"tick broadcast ");

		bc_dump_buf_append(bc_dump_buf,
			"enter counter cpu: ");

		for_each_possible_cpu(i) {
			/* to avoid unexpected overrun */
			if (i >= num_possible_cpus())
				break;
			bc_dump_buf_append(bc_dump_buf, "%lld, ",
				_tick_broadcast_enter_count[i]);
		}

		bc_dump_buf_append(bc_dump_buf,
			"success counter cpu: ");

		for_each_possible_cpu(i) {
			/* to avoid unexpected overrun */
			if (i >= num_possible_cpus())
				break;
			bc_dump_buf_append(bc_dump_buf, "%lld, ",
				_tick_broadcast_success_count[i]);
		}

		bc_dump_buf_append(bc_dump_buf,
			"fail counter cpu: ");

		for_each_possible_cpu(i) {
			/* to avoid unexpected overrun */
			if (i >= num_possible_cpus())
				break;
			bc_dump_buf_append(bc_dump_buf, "%lld, ",
				_tick_broadcast_fail_count[i]);
		}

		bc_dump_buf_append(bc_dump_buf,
			"interrupt counter cpu: ");

		for_each_possible_cpu(i) {
			/* to avoid unexpected overrun */
			if (i >= num_possible_cpus())
				break;
			bc_dump_buf_append(bc_dump_buf, "%lld, ",
				_tick_broadcast_interrupt_count[i]);
		}

		bc_dump_buf_append(bc_dump_buf,
			"o: %*pbl",
			cpumask_pr_args(&_tb_dbg_oneshot_mask));
		bc_dump_buf_append(bc_dump_buf,
			", p: %*pbl",
			cpumask_pr_args(&_tb_dbg_pending_mask));
		bc_dump_buf_append(bc_dump_buf,
			", f: %*pbl",
			cpumask_pr_args(&_tb_dbg_force_mask));
		bc_dump_buf_append(bc_dump_buf,
			", t: ");
		for_each_possible_cpu(i) {
			struct tick_device *td;

			td = &per_cpu(tick_cpu_device, i);
			if (!td || !td->evtdev) {
				if (!td)
					bc_dump_buf_append(bc_dump_buf,
							     " N,");
				else if (!td->evtdev)
					bc_dump_buf_append(bc_dump_buf,
							     " N:%p,", td);
			} else
				bc_dump_buf_append(bc_dump_buf, " %lld,",
					td->evtdev->next_event);
		}
		printk_deferred("[name:bc&]%s\n", get_bc_dump_buf(bc_dump_buf));
	}
#endif
	return ret;
}

/*
 * Reset the one shot broadcast for a cpu
 *
 * Called with tick_broadcast_lock held
 */
static void tick_broadcast_clear_oneshot(int cpu)
{
	cpumask_clear_cpu(cpu, tick_broadcast_oneshot_mask);
	cpumask_clear_cpu(cpu, tick_broadcast_pending_mask);
}

static void tick_broadcast_init_next_event(struct cpumask *mask,
					   ktime_t expires)
{
	struct tick_device *td;
	int cpu;

	for_each_cpu(cpu, mask) {
		td = &per_cpu(tick_cpu_device, cpu);
		if (td->evtdev)
			td->evtdev->next_event = expires;
	}
}

/**
 * tick_broadcast_setup_oneshot - setup the broadcast device
 */
static void tick_broadcast_setup_oneshot(struct clock_event_device *bc)
{
	int cpu = smp_processor_id();

	if (!bc)
		return;

	/* Set it up only once ! */
	if (bc->event_handler != tick_handle_oneshot_broadcast) {
		int was_periodic = clockevent_state_periodic(bc);

		bc->event_handler = tick_handle_oneshot_broadcast;

		/*
		 * We must be careful here. There might be other CPUs
		 * waiting for periodic broadcast. We need to set the
		 * oneshot_mask bits for those and program the
		 * broadcast device to fire.
		 */
		cpumask_copy(tmpmask, tick_broadcast_mask);
		cpumask_clear_cpu(cpu, tmpmask);
		cpumask_or(tick_broadcast_oneshot_mask,
			   tick_broadcast_oneshot_mask, tmpmask);

		if (was_periodic && !cpumask_empty(tmpmask)) {
			clockevents_switch_state(bc, CLOCK_EVT_STATE_ONESHOT);
			tick_broadcast_init_next_event(tmpmask,
						       tick_next_period);
			tick_broadcast_set_event(bc, cpu, tick_next_period);
		} else
			bc->next_event = KTIME_MAX;
	} else {
		/*
		 * The first cpu which switches to oneshot mode sets
		 * the bit for all other cpus which are in the general
		 * (periodic) broadcast mask. So the bit is set and
		 * would prevent the first broadcast enter after this
		 * to program the bc device.
		 */
		tick_broadcast_clear_oneshot(cpu);
	}
}

/*
 * Select oneshot operating mode for the broadcast device
 */
void tick_broadcast_switch_to_oneshot(void)
{
	struct clock_event_device *bc;
	unsigned long flags;

	raw_spin_lock_irqsave(&tick_broadcast_lock, flags);

	tick_broadcast_device.mode = TICKDEV_MODE_ONESHOT;
	bc = tick_broadcast_device.evtdev;
	if (bc)
		tick_broadcast_setup_oneshot(bc);

	raw_spin_unlock_irqrestore(&tick_broadcast_lock, flags);
}

#ifdef CONFIG_HOTPLUG_CPU
void hotplug_cpu__broadcast_tick_pull(int deadcpu)
{
	struct clock_event_device *bc;
	unsigned long flags;
	unsigned int next_cpu;

	raw_spin_lock_irqsave(&tick_broadcast_lock, flags);
	bc = tick_broadcast_device.evtdev;

	if (bc && broadcast_needs_cpu(bc, deadcpu)) {
		/*
		 * If the broadcast force bit of the current CPU is set,
		 * then the current CPU has not yet reprogrammed the local
		 * timer device to avoid a ping-pong race. See
		 * ___tick_broadcast_oneshot_control().
		 *
		 * If the broadcast device is hrtimer based then
		 * programming the broadcast event below does not have any
		 * effect because the local clockevent device is not
		 * running and not programmed because the broadcast event
		 * is not earlier than the pending event of the local clock
		 * event device. As a consequence all CPUs waiting for a
		 * broadcast event are stuck forever.
		 *
		 * Detect this condition and reprogram the cpu local timer
		 * device to avoid the starvation.
		 */
		if (tick_check_broadcast_expired()) {
			struct tick_device *td = this_cpu_ptr(&tick_cpu_device);

			cpumask_clear_cpu(smp_processor_id(), tick_broadcast_force_mask);
			tick_program_event(td->evtdev->next_event, 1);
		}

		/* This moves the broadcast assignment to this CPU: */
		clockevents_program_event(bc, bc->next_event, 1);
	}

	/*
	 * MTK PATCH:
	 *
	 * CPU will not be able to handle irq after shutdown. Change
	 * irq affinity if irq is bound on dying cpu.
	 *
	 * We always set current cpu as new target since finding
	 * real target is lousy and costs time.
	 */
	if (bc && broadcast_needs_cpu_soctimer(bc, deadcpu)) {

		next_cpu = smp_processor_id();
		irq_force_affinity(bc->irq, cpumask_of(next_cpu));
		bc->irq_affinity_on = next_cpu;

		/*
		 * Re-arm soc timer in next_cpu if there was any existed
		 * bctimer armed in dead_cpu to ensure tick-broadcasting
		 * work continuously.
		 *
		 * Do nothing else if bc->next_event is KTIME_MAX here,
		 * for example, in tick_handle_oneshot_broadcast(), if all
		 * cpus' next events are expired, bc->next_event will be set as
		 * KTIME_MAX, and meanwhile cpu hot plug-off event happens.
		 */
		if (bc->next_event != KTIME_MAX)
			clockevents_program_event(bc, bc->next_event, 1);
	}

	raw_spin_unlock_irqrestore(&tick_broadcast_lock, flags);
}

/*
 * Remove a dead CPU from broadcasting
 */
void tick_shutdown_broadcast_oneshot(unsigned int cpu)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&tick_broadcast_lock, flags);

	/*
	 * Clear the broadcast masks for the dead cpu, but do not stop
	 * the broadcast device!
	 */
	cpumask_clear_cpu(cpu, tick_broadcast_oneshot_mask);
	cpumask_clear_cpu(cpu, tick_broadcast_pending_mask);
	cpumask_clear_cpu(cpu, tick_broadcast_force_mask);

	raw_spin_unlock_irqrestore(&tick_broadcast_lock, flags);
}
#endif

/*
 * Check, whether the broadcast device is in one shot mode
 */
int tick_broadcast_oneshot_active(void)
{
	return tick_broadcast_device.mode == TICKDEV_MODE_ONESHOT;
}

/*
 * Check whether the broadcast device supports oneshot.
 */
bool tick_broadcast_oneshot_available(void)
{
	struct clock_event_device *bc = tick_broadcast_device.evtdev;

	return bc ? bc->features & CLOCK_EVT_FEAT_ONESHOT : false;
}

#else
int __tick_broadcast_oneshot_control(enum tick_broadcast_state state)
{
	struct clock_event_device *bc = tick_broadcast_device.evtdev;

	if (!bc || (bc->features & CLOCK_EVT_FEAT_HRTIMER))
		return -EBUSY;

	return 0;
}
#endif

void __init tick_broadcast_init(void)
{
	zalloc_cpumask_var(&tick_broadcast_mask, GFP_NOWAIT);
	zalloc_cpumask_var(&tick_broadcast_on, GFP_NOWAIT);
	zalloc_cpumask_var(&tmpmask, GFP_NOWAIT);
#ifdef CONFIG_TICK_ONESHOT
	zalloc_cpumask_var(&tick_broadcast_oneshot_mask, GFP_NOWAIT);
	zalloc_cpumask_var(&tick_broadcast_pending_mask, GFP_NOWAIT);
	zalloc_cpumask_var(&tick_broadcast_force_mask, GFP_NOWAIT);
#endif
}
