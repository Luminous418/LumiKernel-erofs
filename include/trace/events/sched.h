/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM sched

#if !defined(_TRACE_SCHED_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_SCHED_H

#include <linux/kthread.h>
#include <linux/sched/numa_balancing.h>
#include <linux/tracepoint.h>
#include <linux/binfmts.h>

#ifdef CONFIG_MTK_SCHED_TRACERS
/* M: states for tracking I/O & mutex events
 * notice avoid to conflict with linux/sched.h
 *
 * A bug linux not fixed:
 * 'K' for TASK_WAKEKILL specified in linux/sched.h
 * but marked 'K' in sched_switch will cause Android systrace parser confused
 * therefore for sched_switch events, these extra states will be printed
 * in the end of each line
 */
#define _MT_TASK_BLOCKED_RTMUX		(TASK_STATE_MAX << 1)
#define _MT_TASK_BLOCKED_MUTEX		(TASK_STATE_MAX << 2)
#define _MT_TASK_BLOCKED_IO		(TASK_STATE_MAX << 3)
#define _MT_EXTRA_STATE_MASK (_MT_TASK_BLOCKED_RTMUX | \
			      _MT_TASK_BLOCKED_MUTEX | \
			      _MT_TASK_BLOCKED_IO | \
			      TASK_WAKEKILL | \
			      TASK_PARKED | \
			      TASK_NOLOAD)
#endif
#define _MT_TASK_STATE_MASK  ((TASK_STATE_MAX - 1) & \
			      ~(TASK_WAKEKILL | TASK_PARKED | TASK_NOLOAD))
/*
 * Tracepoint for calling kthread_stop, performed to end a kthread:
 */
TRACE_EVENT(sched_kthread_stop,

	TP_PROTO(struct task_struct *t),

	TP_ARGS(t),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, t->comm, TASK_COMM_LEN);
		__entry->pid	= t->pid;
	),

	TP_printk("comm=%s pid=%d", __entry->comm, __entry->pid)
);

/*
 * Tracepoint for the return value of the kthread stopping:
 */
TRACE_EVENT(sched_kthread_stop_ret,

	TP_PROTO(int ret),

	TP_ARGS(ret),

	TP_STRUCT__entry(
		__field(	int,	ret	)
	),

	TP_fast_assign(
		__entry->ret	= ret;
	),

	TP_printk("ret=%d", __entry->ret)
);

#ifdef CREATE_TRACE_POINTS
static inline long __trace_sched_switch_state(bool preempt,
						struct task_struct *p);
#endif
/**
 * sched_kthread_work_queue_work - called when a work gets queued
 * @worker:	pointer to the kthread_worker
 * @work:	pointer to struct kthread_work
 *
 * This event occurs when a work is queued immediately or once a
 * delayed work is actually queued (ie: once the delay has been
 * reached).
 */
TRACE_EVENT(sched_kthread_work_queue_work,

	TP_PROTO(struct kthread_worker *worker,
		 struct kthread_work *work),

	TP_ARGS(worker, work),

	TP_STRUCT__entry(
		__field( void *,	work	)
		__field( void *,	function)
		__field( void *,	worker)
	),

	TP_fast_assign(
		__entry->work		= work;
		__entry->function	= work->func;
		__entry->worker		= worker;
	),

	TP_printk("work struct=%p function=%ps worker=%p",
		  __entry->work, __entry->function, __entry->worker)
);

/**
 * sched_kthread_work_execute_start - called immediately before the work callback
 * @work:	pointer to struct kthread_work
 *
 * Allows to track kthread work execution.
 */
TRACE_EVENT(sched_kthread_work_execute_start,

	TP_PROTO(struct kthread_work *work),

	TP_ARGS(work),

	TP_STRUCT__entry(
		__field( void *,	work	)
		__field( void *,	function)
	),

	TP_fast_assign(
		__entry->work		= work;
		__entry->function	= work->func;
	),

	TP_printk("work struct %p: function %ps", __entry->work, __entry->function)
);

/**
 * sched_kthread_work_execute_end - called immediately after the work callback
 * @work:	pointer to struct work_struct
 * @function:   pointer to worker function
 *
 * Allows to track workqueue execution.
 */
TRACE_EVENT(sched_kthread_work_execute_end,

	TP_PROTO(struct kthread_work *work, kthread_work_func_t function),

	TP_ARGS(work, function),

	TP_STRUCT__entry(
		__field( void *,	work	)
		__field( void *,	function)
	),

	TP_fast_assign(
		__entry->work		= work;
		__entry->function	= function;
	),

	TP_printk("work struct %p: function %ps", __entry->work, __entry->function)
);

/*
 * Tracepoint for waking up a task:
 */
DECLARE_EVENT_CLASS(sched_wakeup_template,

	TP_PROTO(struct task_struct *p),

	TP_ARGS(__perf_task(p)),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
		__field(	int,	success			)
		__field(	int,	target_cpu		)
#ifdef CONFIG_MTK_SCHED_TRACERS
		__field(long, state)
#endif
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio; /* XXX SCHED_DEADLINE */
		__entry->success	= 1; /* rudiment, kill when possible */
		__entry->target_cpu	= task_cpu(p);
#ifdef CONFIG_MTK_SCHED_TRACERS
		__entry->state	= __trace_sched_switch_state(false, p);
#endif
	),
#ifdef CONFIG_MTK_SCHED_TRACERS
	TP_printk(
	"comm=%s pid=%d prio=%d success=%d target_cpu=%03d state=%s",
		  __entry->comm, __entry->pid, __entry->prio,
		  __entry->success, __entry->target_cpu,
		  __entry->state & (~TASK_STATE_MAX) ?
		  __print_flags(__entry->state & (~TASK_STATE_MAX), "|",
				{ TASK_INTERRUPTIBLE, "S"},
				{ TASK_UNINTERRUPTIBLE, "D"},
				{ __TASK_STOPPED, "T"},
				{ __TASK_TRACED, "t"},
				{ EXIT_ZOMBIE, "Z"},
				{ EXIT_DEAD, "X"},
				{ TASK_DEAD, "x"},
				{ TASK_WAKEKILL, "K"},
				{ TASK_WAKING, "W"},
				{ TASK_PARKED, "P"},
				{ TASK_NOLOAD, "N"},
				{ _MT_TASK_BLOCKED_RTMUX, "r"},
				{ _MT_TASK_BLOCKED_MUTEX, "m"},
				{ _MT_TASK_BLOCKED_IO, "d"}) : "R")
#else
	TP_printk("comm=%s pid=%d prio=%d target_cpu=%03d",
		  __entry->comm, __entry->pid, __entry->prio,
		  __entry->target_cpu)
#endif
);

/*
 * Tracepoint called when waking a task; this tracepoint is guaranteed to be
 * called from the waking context.
 */
DEFINE_EVENT(sched_wakeup_template, sched_waking,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

/*
 * Tracepoint called when the task is actually woken; p->state == TASK_RUNNNG.
 * It it not always called from the waking context.
 */
DEFINE_EVENT(sched_wakeup_template, sched_wakeup,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

/*
 * Tracepoint for waking up a new task:
 */
DEFINE_EVENT(sched_wakeup_template, sched_wakeup_new,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

#ifdef CREATE_TRACE_POINTS
static inline long __trace_sched_switch_state(bool preempt, struct task_struct *p)
{
#ifdef CONFIG_MTK_SCHED_TRACERS
	long state = p->state;
	/*
	 * M:mark as comment to export more task state for
	 * migration & wakeup
	 */
#else
	unsigned int state;
#endif

#ifdef CONFIG_SCHED_DEBUG
	//BUG_ON(p != current);
#endif /* CONFIG_SCHED_DEBUG */

	/*
	 * Preemption ignores task state, therefore preempted tasks are always
	 * RUNNING (we will not have dequeued if state != RUNNING).
	 */
#ifdef CONFIG_MTK_SCHED_TRACERS
	if (preempt)
		state = TASK_RUNNING | TASK_STATE_MAX;
#ifdef CONFIG_RT_MUTEXES
	if (p->pi_blocked_on)
		state |= _MT_TASK_BLOCKED_RTMUX;
#endif
#ifdef CONFIG_DEBUG_MUTEXES
	if (p->blocked_on)
		state |= _MT_TASK_BLOCKED_MUTEX;
#endif
	if ((p->state & TASK_UNINTERRUPTIBLE) && p->in_iowait)
		state |= _MT_TASK_BLOCKED_IO;

	return state;
#else
	if (preempt)
		return TASK_REPORT_MAX;
	/*
	 * task_state_index() uses fls() and returns a value from 0-8 range.
	 * Decrement it by 1 (except TASK_RUNNING state i.e 0) before using
	 * it for left shift operation to get the correct task->state
	 * mapping.
	 */
	state = __get_task_state(p);

	return state ? (1 << (state - 1)) : state;
#endif
}
#endif /* CREATE_TRACE_POINTS */

/*
 * Tracepoint for task switches, performed by the scheduler:
 */
TRACE_EVENT(sched_switch,

	TP_PROTO(bool preempt,
		 struct task_struct *prev,
		 struct task_struct *next),

	TP_ARGS(preempt, prev, next),

	TP_STRUCT__entry(
		__array(	char,	prev_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	prev_pid			)
		__field(	int,	prev_prio			)
		__field(	long,	prev_state			)
		__array(	char,	next_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	next_pid			)
		__field(	int,	next_prio			)
#if defined(CONFIG_MTK_SCHED_TRACERS) && defined(CONFIG_CGROUPS)
		__field(int,	prev_cgrp_id)
		__field(int,	next_cgrp_id)
		__field(int,	prev_st_cgrp_id)
		__field(int,	next_st_cgrp_id)
#endif
	),

	TP_fast_assign(
		memcpy(__entry->next_comm, next->comm, TASK_COMM_LEN);
		__entry->prev_pid	= prev->pid;
		__entry->prev_prio	= prev->prio;
		__entry->prev_state	= __trace_sched_switch_state(preempt, prev);
		memcpy(__entry->prev_comm, prev->comm, TASK_COMM_LEN);
		__entry->next_pid	= next->pid;
		__entry->next_prio	= next->prio;
#if defined(CONFIG_MTK_SCHED_TRACERS) && defined(CONFIG_CGROUPS)
#if defined(CONFIG_CPUSETS)
		__entry->prev_cgrp_id	= prev->cgroups->subsys[0]->cgroup->id;
		__entry->next_cgrp_id	= next->cgroups->subsys[0]->cgroup->id;
#else
		__entry->prev_cgrp_id	= 0;
		__entry->next_cgrp_id	= 0;
#endif
#if defined(CONFIG_SCHED_TUNE)
		__entry->prev_st_cgrp_id = prev->cgroups->subsys[3]->cgroup->id;
		__entry->next_st_cgrp_id = next->cgroups->subsys[3]->cgroup->id;
#else
		__entry->prev_st_cgrp_id = 0;
		__entry->next_st_cgrp_id = 0;
#endif
#endif
		/* XXX SCHED_DEADLINE */
	),
#ifdef CONFIG_MTK_SCHED_TRACERS
	TP_printk(
#if defined(CONFIG_CGROUPS)
	"prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d%s%s prev->cgrp=%d next->cgrp=%d prev->st=%d next->st=%d",
#else
	"prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d%s%s",
#endif
		__entry->prev_comm, __entry->prev_pid, __entry->prev_prio,
		__entry->prev_state & (_MT_TASK_STATE_MASK) ?
		__print_flags(__entry->prev_state & (_MT_TASK_STATE_MASK), "|",
				{ TASK_INTERRUPTIBLE, "S" },
				{ TASK_UNINTERRUPTIBLE, "D" },
				{ __TASK_STOPPED, "T" },
				{ __TASK_TRACED, "t" },
				{ EXIT_DEAD, "X" },
				{ EXIT_ZOMBIE, "Z" },
				{ TASK_DEAD, "x" },
				{ TASK_WAKEKILL, "K"},
				{ TASK_WAKING, "W"}) : "R",
		__entry->prev_state & TASK_STATE_MAX ? "+" : "",
		__entry->next_comm, __entry->next_pid, __entry->next_prio,
		(__entry->prev_state & _MT_EXTRA_STATE_MASK) ?
			" extra_prev_state=" : "",
		__print_flags(__entry->prev_state & _MT_EXTRA_STATE_MASK, "|",
				{ TASK_WAKEKILL, "K" },
				{ TASK_PARKED, "P" },
				{ TASK_NOLOAD, "N" },
				{ _MT_TASK_BLOCKED_RTMUX, "r" },
				{ _MT_TASK_BLOCKED_MUTEX, "m" },
				{ _MT_TASK_BLOCKED_IO, "d" })
#if defined(CONFIG_CGROUPS)
				, __entry->prev_cgrp_id
				, __entry->next_cgrp_id
				, __entry->prev_st_cgrp_id
				, __entry->next_st_cgrp_id
#endif
	)
#else

	TP_printk("prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d",
		__entry->prev_comm, __entry->prev_pid, __entry->prev_prio,

		(__entry->prev_state & (TASK_REPORT_MAX - 1)) ?
		  __print_flags(__entry->prev_state & (TASK_REPORT_MAX - 1), "|",
				{ TASK_INTERRUPTIBLE, "S" },
				{ TASK_UNINTERRUPTIBLE, "D" },
				{ __TASK_STOPPED, "T" },
				{ __TASK_TRACED, "t" },
				{ EXIT_DEAD, "X" },
				{ EXIT_ZOMBIE, "Z" },
				{ TASK_PARKED, "P" },
				{ TASK_DEAD, "I" }) :
		  "R",

		__entry->prev_state & TASK_REPORT_MAX ? "+" : "",
		__entry->next_comm, __entry->next_pid, __entry->next_prio)
#endif
);

/*
 * Tracepoint for a task being migrated:
 */
TRACE_EVENT(sched_migrate_task,

	TP_PROTO(struct task_struct *p, int dest_cpu),

	TP_ARGS(p, dest_cpu),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
		__field(	int,	orig_cpu		)
		__field(	int,	dest_cpu		)
#ifdef CONFIG_MTK_SCHED_TRACERS
		__field(long, state)
#endif
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio; /* XXX SCHED_DEADLINE */
		__entry->orig_cpu	= task_cpu(p);
		__entry->dest_cpu	= dest_cpu;
#ifdef CONFIG_MTK_SCHED_TRACERS
		__entry->state      =	__trace_sched_switch_state(false, p);
#endif
	),

#ifdef CONFIG_MTK_SCHED_TRACERS
	TP_printk("comm=%s pid=%d prio=%d orig_cpu=%d dest_cpu=%d state=%s",
		__entry->comm, __entry->pid, __entry->prio,
		__entry->orig_cpu, __entry->dest_cpu,
		__entry->state & (~TASK_STATE_MAX) ?
		__print_flags(__entry->state & (~TASK_STATE_MAX), "|",
				{ TASK_INTERRUPTIBLE, "S"},
				{ TASK_UNINTERRUPTIBLE, "D" },
				{ __TASK_STOPPED, "T" },
				{ __TASK_TRACED, "t" },
				{ EXIT_ZOMBIE, "Z" },
				{ EXIT_DEAD, "X" },
				{ TASK_DEAD, "x" },
				{ TASK_WAKEKILL, "K" },
				{ TASK_WAKING, "W" },
				{ TASK_PARKED, "P" },
				{ TASK_NOLOAD, "N" },
				{ _MT_TASK_BLOCKED_RTMUX, "r" },
				{ _MT_TASK_BLOCKED_MUTEX, "m"},
				{ _MT_TASK_BLOCKED_IO, "d"}) : "R")
#else
	TP_printk("comm=%s pid=%d prio=%d orig_cpu=%d dest_cpu=%d",
		  __entry->comm, __entry->pid, __entry->prio,
		  __entry->orig_cpu, __entry->dest_cpu)
#endif
);

DECLARE_EVENT_CLASS(sched_process_template,

	TP_PROTO(struct task_struct *p),

	TP_ARGS(p),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio; /* XXX SCHED_DEADLINE */
	),

	TP_printk("comm=%s pid=%d prio=%d",
		  __entry->comm, __entry->pid, __entry->prio)
);

/*
 * Tracepoint for freeing a task:
 */
DEFINE_EVENT(sched_process_template, sched_process_free,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));


/*
 * Tracepoint for a task exiting:
 */
DEFINE_EVENT(sched_process_template, sched_process_exit,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

/*
 * Tracepoint for waiting on task to unschedule:
 */
DEFINE_EVENT(sched_process_template, sched_wait_task,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p));

/*
 * Tracepoint for a waiting task:
 */
TRACE_EVENT(sched_process_wait,

	TP_PROTO(struct pid *pid),

	TP_ARGS(pid),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
		__entry->pid		= pid_nr(pid);
		__entry->prio		= current->prio; /* XXX SCHED_DEADLINE */
	),

	TP_printk("comm=%s pid=%d prio=%d",
		  __entry->comm, __entry->pid, __entry->prio)
);

/*
 * Tracepoint for do_fork:
 */
TRACE_EVENT(sched_process_fork,

	TP_PROTO(struct task_struct *parent, struct task_struct *child),

	TP_ARGS(parent, child),

	TP_STRUCT__entry(
		__array(	char,	parent_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	parent_pid			)
		__array(	char,	child_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	child_pid			)
	),

	TP_fast_assign(
		memcpy(__entry->parent_comm, parent->comm, TASK_COMM_LEN);
		__entry->parent_pid	= parent->pid;
		memcpy(__entry->child_comm, child->comm, TASK_COMM_LEN);
		__entry->child_pid	= child->pid;
	),

	TP_printk("comm=%s pid=%d child_comm=%s child_pid=%d",
		__entry->parent_comm, __entry->parent_pid,
		__entry->child_comm, __entry->child_pid)
);

/*
 * Tracepoint for exec:
 */
TRACE_EVENT(sched_process_exec,

	TP_PROTO(struct task_struct *p, pid_t old_pid,
		 struct linux_binprm *bprm),

	TP_ARGS(p, old_pid, bprm),

	TP_STRUCT__entry(
		__string(	filename,	bprm->filename	)
		__field(	pid_t,		pid		)
		__field(	pid_t,		old_pid		)
	),

	TP_fast_assign(
		__assign_str(filename, bprm->filename);
		__entry->pid		= p->pid;
		__entry->old_pid	= old_pid;
	),

	TP_printk("filename=%s pid=%d old_pid=%d", __get_str(filename),
		  __entry->pid, __entry->old_pid)
);

/*
 * XXX the below sched_stat tracepoints only apply to SCHED_OTHER/BATCH/IDLE
 *     adding sched_stat support to SCHED_FIFO/RR would be welcome.
 */
DECLARE_EVENT_CLASS(sched_stat_template,

	TP_PROTO(struct task_struct *tsk, u64 delay),

	TP_ARGS(__perf_task(tsk), __perf_count(delay)),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( u64,	delay			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid	= tsk->pid;
		__entry->delay	= delay;
	),

	TP_printk("comm=%s pid=%d delay=%Lu [ns]",
			__entry->comm, __entry->pid,
			(unsigned long long)__entry->delay)
);

/*
 * Tracepoint for schedutil governor
 */
TRACE_EVENT(sched_util,
	TP_PROTO(int cid, unsigned int next_freq, u64 time),
	TP_ARGS(cid, next_freq, time),
	TP_STRUCT__entry(
		__field(int, cid)
		__field(unsigned int, next_freq)
		__field(u64, time)
	),
	TP_fast_assign(
		__entry->cid		= cid;
		__entry->next_freq	= next_freq;
		__entry->time		= time;
	),
	TP_printk("cid=%d next=%u last_freq_update_time=%lld",
		__entry->cid,
		__entry->next_freq,
		__entry->time
	)
);

/*
 * Tracepoint for accounting wait time (time the task is runnable
 * but not actually running due to scheduler contention).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_wait,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting sleep time (time the task is not runnable,
 * including iowait, see below).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_sleep,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting iowait time (time the task is not runnable
 * due to waiting on IO to complete).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_iowait,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting blocked time (time the task is in uninterruptible).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_blocked,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for recording the cause of uninterruptible sleep.
 */
TRACE_EVENT(sched_blocked_reason,

	TP_PROTO(struct task_struct *tsk),

	TP_ARGS(tsk),

	TP_STRUCT__entry(
		__field( pid_t,	pid	)
		__field( void*, caller	)
		__field( bool, io_wait	)
	),

	TP_fast_assign(
		__entry->pid	= tsk->pid;
		__entry->caller = (void*)get_wchan(tsk);
		__entry->io_wait = tsk->in_iowait;
	),

	TP_printk("pid=%d iowait=%d caller=%pS", __entry->pid, __entry->io_wait, __entry->caller)
);

/*
 * Tracepoint for accounting runtime (time the task is executing
 * on a CPU).
 */
DECLARE_EVENT_CLASS(sched_stat_runtime,

	TP_PROTO(struct task_struct *tsk, u64 runtime, u64 vruntime),

	TP_ARGS(tsk, __perf_count(runtime), vruntime),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( u64,	runtime			)
		__field( u64,	vruntime			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->runtime	= runtime;
		__entry->vruntime	= vruntime;
	),

	TP_printk("comm=%s pid=%d runtime=%Lu [ns] vruntime=%Lu [ns]",
			__entry->comm, __entry->pid,
			(unsigned long long)__entry->runtime,
			(unsigned long long)__entry->vruntime)
);

DEFINE_EVENT(sched_stat_runtime, sched_stat_runtime,
	     TP_PROTO(struct task_struct *tsk, u64 runtime, u64 vruntime),
	     TP_ARGS(tsk, runtime, vruntime));

/*
 * Tracepoint for showing priority inheritance modifying a tasks
 * priority.
 */
TRACE_EVENT(sched_pi_setprio,

	TP_PROTO(struct task_struct *tsk, struct task_struct *pi_task),

	TP_ARGS(tsk, pi_task),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( int,	oldprio			)
		__field( int,	newprio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->oldprio	= tsk->prio;
		__entry->newprio	= pi_task ?
				min(tsk->normal_prio, pi_task->prio) :
				tsk->normal_prio;
		/* XXX SCHED_DEADLINE bits missing */
	),

	TP_printk("comm=%s pid=%d oldprio=%d newprio=%d",
			__entry->comm, __entry->pid,
			__entry->oldprio, __entry->newprio)
);

#ifdef CONFIG_DETECT_HUNG_TASK
TRACE_EVENT(sched_process_hang,
	TP_PROTO(struct task_struct *tsk),
	TP_ARGS(tsk),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid = tsk->pid;
	),

	TP_printk("comm=%s pid=%d", __entry->comm, __entry->pid)
);
#endif /* CONFIG_DETECT_HUNG_TASK */

DECLARE_EVENT_CLASS(sched_move_task_template,

	TP_PROTO(struct task_struct *tsk, int src_cpu, int dst_cpu),

	TP_ARGS(tsk, src_cpu, dst_cpu),

	TP_STRUCT__entry(
		__field( pid_t,	pid			)
		__field( pid_t,	tgid			)
		__field( pid_t,	ngid			)
		__field( int,	src_cpu			)
		__field( int,	src_nid			)
		__field( int,	dst_cpu			)
		__field( int,	dst_nid			)
	),

	TP_fast_assign(
		__entry->pid		= task_pid_nr(tsk);
		__entry->tgid		= task_tgid_nr(tsk);
		__entry->ngid		= task_numa_group_id(tsk);
		__entry->src_cpu	= src_cpu;
		__entry->src_nid	= cpu_to_node(src_cpu);
		__entry->dst_cpu	= dst_cpu;
		__entry->dst_nid	= cpu_to_node(dst_cpu);
	),

	TP_printk("pid=%d tgid=%d ngid=%d src_cpu=%d src_nid=%d dst_cpu=%d dst_nid=%d",
			__entry->pid, __entry->tgid, __entry->ngid,
			__entry->src_cpu, __entry->src_nid,
			__entry->dst_cpu, __entry->dst_nid)
);

/*
 * Tracks migration of tasks from one runqueue to another. Can be used to
 * detect if automatic NUMA balancing is bouncing between nodes
 */
DEFINE_EVENT(sched_move_task_template, sched_move_numa,
	TP_PROTO(struct task_struct *tsk, int src_cpu, int dst_cpu),

	TP_ARGS(tsk, src_cpu, dst_cpu)
);

DEFINE_EVENT(sched_move_task_template, sched_stick_numa,
	TP_PROTO(struct task_struct *tsk, int src_cpu, int dst_cpu),

	TP_ARGS(tsk, src_cpu, dst_cpu)
);

TRACE_EVENT(sched_swap_numa,

	TP_PROTO(struct task_struct *src_tsk, int src_cpu,
		 struct task_struct *dst_tsk, int dst_cpu),

	TP_ARGS(src_tsk, src_cpu, dst_tsk, dst_cpu),

	TP_STRUCT__entry(
		__field( pid_t,	src_pid			)
		__field( pid_t,	src_tgid		)
		__field( pid_t,	src_ngid		)
		__field( int,	src_cpu			)
		__field( int,	src_nid			)
		__field( pid_t,	dst_pid			)
		__field( pid_t,	dst_tgid		)
		__field( pid_t,	dst_ngid		)
		__field( int,	dst_cpu			)
		__field( int,	dst_nid			)
	),

	TP_fast_assign(
		__entry->src_pid	= task_pid_nr(src_tsk);
		__entry->src_tgid	= task_tgid_nr(src_tsk);
		__entry->src_ngid	= task_numa_group_id(src_tsk);
		__entry->src_cpu	= src_cpu;
		__entry->src_nid	= cpu_to_node(src_cpu);
		__entry->dst_pid	= task_pid_nr(dst_tsk);
		__entry->dst_tgid	= task_tgid_nr(dst_tsk);
		__entry->dst_ngid	= task_numa_group_id(dst_tsk);
		__entry->dst_cpu	= dst_cpu;
		__entry->dst_nid	= cpu_to_node(dst_cpu);
	),

	TP_printk("src_pid=%d src_tgid=%d src_ngid=%d src_cpu=%d src_nid=%d dst_pid=%d dst_tgid=%d dst_ngid=%d dst_cpu=%d dst_nid=%d",
			__entry->src_pid, __entry->src_tgid, __entry->src_ngid,
			__entry->src_cpu, __entry->src_nid,
			__entry->dst_pid, __entry->dst_tgid, __entry->dst_ngid,
			__entry->dst_cpu, __entry->dst_nid)
);

/*
 * Tracepoint for waking a polling cpu without an IPI.
 */
TRACE_EVENT(sched_wake_idle_without_ipi,

	TP_PROTO(int cpu),

	TP_ARGS(cpu),

	TP_STRUCT__entry(
		__field(	int,	cpu	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
	),

	TP_printk("cpu=%d", __entry->cpu)
);

#ifdef CONFIG_SMP
#ifdef CREATE_TRACE_POINTS
static inline
int __trace_sched_cpu(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
#ifdef CONFIG_FAIR_GROUP_SCHED
	struct rq *rq = cfs_rq ? cfs_rq->rq : NULL;
#else
	struct rq *rq = cfs_rq ? container_of(cfs_rq, struct rq, cfs) : NULL;
#endif
	return rq ? cpu_of(rq)
		  : task_cpu((container_of(se, struct task_struct, se)));
}

static inline
int __trace_sched_path(struct cfs_rq *cfs_rq, char *path, int len)
{
#ifdef CONFIG_FAIR_GROUP_SCHED
	int l = path ? len : 0;

	if (cfs_rq && task_group_is_autogroup(cfs_rq->tg))
		return autogroup_path(cfs_rq->tg, path, l) + 1;
	else if (cfs_rq && cfs_rq->tg->css.cgroup)
		return cgroup_path(cfs_rq->tg->css.cgroup, path, l) + 1;
#endif
	if (path)
		strcpy(path, "(null)");

	return strlen("(null)");
}

static inline
struct cfs_rq *__trace_sched_group_cfs_rq(struct sched_entity *se)
{
#ifdef CONFIG_FAIR_GROUP_SCHED
	return se->my_q;
#else
	return NULL;
#endif
}
#endif /* CREATE_TRACE_POINTS */

#ifdef CONFIG_SCHED_WALT
extern unsigned int sysctl_sched_use_walt_cpu_util;
extern unsigned int sysctl_sched_use_walt_task_util;
extern unsigned int walt_ravg_window;
extern bool walt_disabled;

#define walt_util(util_var, demand_sum) {\
	u64 sum = demand_sum << SCHED_CAPACITY_SHIFT;\
	do_div(sum, walt_ravg_window);\
	util_var = (typeof(util_var))sum;\
	}
#endif

/*
 * Tracepoint for cfs_rq load tracking:
 */
TRACE_EVENT(sched_load_cfs_rq,

	TP_PROTO(struct cfs_rq *cfs_rq),

	TP_ARGS(cfs_rq),

	TP_STRUCT__entry(
		__field(	int,		cpu			)
		__dynamic_array(char,		path,
				__trace_sched_path(cfs_rq, NULL, 0)	)
		__field(	unsigned long,	load			)
		__field(	unsigned long,	util			)
		__field(	unsigned long,	util_pelt          	)
		__field(	unsigned long,	util_walt          	)
	),

	TP_fast_assign(
		__entry->cpu	= __trace_sched_cpu(cfs_rq, NULL);
		__trace_sched_path(cfs_rq, __get_dynamic_array(path),
				   __get_dynamic_array_len(path));
		__entry->load	= cfs_rq->runnable_load_avg;
		__entry->util	= cfs_rq->avg.util_avg;
		__entry->util_pelt = cfs_rq->avg.util_avg;
		__entry->util_walt = 0;
#ifdef CONFIG_SCHED_WALT
		if (&cfs_rq->rq->cfs == cfs_rq) {
			walt_util(__entry->util_walt,
				  cfs_rq->rq->prev_runnable_sum);
			if (!walt_disabled && sysctl_sched_use_walt_cpu_util)
				__entry->util = __entry->util_walt;
		}
#endif
	),

	TP_printk("cpu=%d path=%s load=%lu util=%lu util_pelt=%lu util_walt=%lu",
		  __entry->cpu, __get_str(path), __entry->load, __entry->util,
		  __entry->util_pelt, __entry->util_walt)
);

/*
 * Tracepoint for rt_rq load tracking:
 */
struct rt_rq;

TRACE_EVENT(sched_load_rt_rq,

	TP_PROTO(int cpu, struct rt_rq *rt_rq),

	TP_ARGS(cpu, rt_rq),

	TP_STRUCT__entry(
		__field(	int,		cpu			)
		__field(	unsigned long,	util			)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
		__entry->util	= rt_rq->avg.util_avg;
	),

	TP_printk("cpu=%d util=%lu", __entry->cpu,
		  __entry->util)
);

/*
 * Tracepoint for sched_entity load tracking:
 */
TRACE_EVENT(sched_load_se,

	TP_PROTO(struct sched_entity *se),

	TP_ARGS(se),

	TP_STRUCT__entry(
		__field(	int,		cpu			      )
		__dynamic_array(char,		path,
		  __trace_sched_path(__trace_sched_group_cfs_rq(se), NULL, 0) )
		__array(	char,		comm,	TASK_COMM_LEN	      )
		__field(	pid_t,		pid			      )
		__field(	unsigned long,	load			      )
		__field(	unsigned long,	util			      )
		__field(	unsigned long,	util_pelt		      )
		__field(	unsigned long,	util_walt		      )
	),

	TP_fast_assign(
		struct cfs_rq *gcfs_rq = __trace_sched_group_cfs_rq(se);
		struct task_struct *p = gcfs_rq ? NULL
				    : container_of(se, struct task_struct, se);

		__entry->cpu = __trace_sched_cpu(gcfs_rq, se);
		__trace_sched_path(gcfs_rq, __get_dynamic_array(path),
				   __get_dynamic_array_len(path));
		memcpy(__entry->comm, p ? p->comm : "(null)",
				      p ? TASK_COMM_LEN : sizeof("(null)"));
		__entry->pid = p ? p->pid : -1;
		__entry->load = se->avg.load_avg;
		__entry->util = se->avg.util_avg;
		__entry->util_pelt  = __entry->util;
		__entry->util_walt  = 0;
#ifdef CONFIG_SCHED_WALT
		if (!se->my_q) {
			struct task_struct *p = container_of(se, struct task_struct, se);
			walt_util(__entry->util_walt, p->ravg.demand);
			if (!walt_disabled && sysctl_sched_use_walt_task_util)
				__entry->util = __entry->util_walt;
		}
#endif
	),

	TP_printk("cpu=%d path=%s comm=%s pid=%d load=%lu util=%lu util_pelt=%lu util_walt=%lu",
		  __entry->cpu, __get_str(path), __entry->comm,
		  __entry->pid, __entry->load, __entry->util,
		  __entry->util_pelt, __entry->util_walt)
);

/*
 * Tracepoint for task_group load tracking:
 */
#ifdef CONFIG_FAIR_GROUP_SCHED
TRACE_EVENT(sched_load_tg,

	TP_PROTO(struct cfs_rq *cfs_rq),

	TP_ARGS(cfs_rq),

	TP_STRUCT__entry(
		__field(	int,	cpu				)
		__dynamic_array(char,	path,
				__trace_sched_path(cfs_rq, NULL, 0)	)
		__field(	long,	load				)
	),

	TP_fast_assign(
		__entry->cpu	= cfs_rq->rq->cpu;
		__trace_sched_path(cfs_rq, __get_dynamic_array(path),
				   __get_dynamic_array_len(path));
		__entry->load	= atomic_long_read(&cfs_rq->tg->load_avg);
	),

	TP_printk("cpu=%d path=%s load=%ld", __entry->cpu, __get_str(path),
		  __entry->load)
);
#endif /* CONFIG_FAIR_GROUP_SCHED */

/*
 * Tracepoint for accounting CPU  boosted utilization
 */
TRACE_EVENT(sched_boost_cpu,

	TP_PROTO(int cpu, unsigned long util, long margin),

	TP_ARGS(cpu, util, margin),

	TP_STRUCT__entry(
		__field( int,		cpu			)
		__field( unsigned long,	util			)
		__field(long,		margin			)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
		__entry->util	= util;
		__entry->margin	= margin;
	),

	TP_printk("cpu=%d util=%lu margin=%ld",
		  __entry->cpu,
		  __entry->util,
		  __entry->margin)
);

/*
 * Tracepoint for schedtune_tasks_update
 */
TRACE_EVENT(sched_tune_tasks_update,

	TP_PROTO(struct task_struct *tsk, int cpu, int tasks, int idx,
		int boost, int max_boost, u64 group_ts),

	TP_ARGS(tsk, cpu, tasks, idx, boost, max_boost, group_ts),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,		pid		)
		__field( int,		cpu		)
		__field( int,		tasks		)
		__field( int,		idx		)
		__field( int,		boost		)
		__field( int,		max_boost	)
		__field( u64,		group_ts	)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->cpu 		= cpu;
		__entry->tasks		= tasks;
		__entry->idx 		= idx;
		__entry->boost		= boost;
		__entry->max_boost	= max_boost;
		__entry->group_ts	= group_ts;
	),

	TP_printk("pid=%d comm=%s "
			"cpu=%d tasks=%d idx=%d boost=%d max_boost=%d timeout=%llu",
		__entry->pid, __entry->comm,
		__entry->cpu, __entry->tasks, __entry->idx,
		__entry->boost, __entry->max_boost,
		__entry->group_ts)
);

/*
 * Tracepoint for schedtune_boostgroup_update
 */
TRACE_EVENT(sched_tune_boostgroup_update,

	TP_PROTO(int cpu, int variation, int max_boost),

	TP_ARGS(cpu, variation, max_boost),

	TP_STRUCT__entry(
		__field( int,	cpu		)
		__field( int,	variation	)
		__field( int,	max_boost	)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->variation	= variation;
		__entry->max_boost	= max_boost;
	),

	TP_printk("cpu=%d variation=%d max_boost=%d",
		__entry->cpu, __entry->variation, __entry->max_boost)
);

/*
 * Tracepoint for accounting task boosted utilization
 */
TRACE_EVENT(sched_boost_task,

	TP_PROTO(struct task_struct *tsk, unsigned long util, long margin,
		unsigned int util_min),

	TP_ARGS(tsk, util, margin, util_min),

	TP_STRUCT__entry(
		__array(char,	comm,	TASK_COMM_LEN)
		__field(pid_t,		pid)
		__field(unsigned long,	util)
		__field(long,		margin)
		__field(unsigned int,	util_min)

	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid	= tsk->pid;
		__entry->util	= util;
		__entry->margin	= margin;
		__entry->util_min = util_min;
	),

	TP_printk("comm=%s pid=%d util=%lu margin=%ld util_min=%u",
		  __entry->comm, __entry->pid,
		  __entry->util,
		  __entry->margin,
		  __entry->util_min)
);

/*
 * Tracepoint for system overutilized flag
 */
struct sched_domain;
TRACE_EVENT_CONDITION(sched_overutilized,

	TP_PROTO(struct sched_domain *sd, bool was_overutilized, bool overutilized),

	TP_ARGS(sd, was_overutilized, overutilized),

	TP_CONDITION(overutilized != was_overutilized),

	TP_STRUCT__entry(
		__field( bool,	overutilized	  )
		__array( char,  cpulist , 32      )
	),

	TP_fast_assign(
		__entry->overutilized	= overutilized;
		scnprintf(__entry->cpulist, sizeof(__entry->cpulist), "%*pbl", cpumask_pr_args(sched_domain_span(sd)));
	),

	TP_printk("overutilized=%d sd_span=%s",
		__entry->overutilized ? 1 : 0, __entry->cpulist)
);

/*
 * Tracepoint for find_best_target
 */
TRACE_EVENT(sched_find_best_target,

	TP_PROTO(struct task_struct *tsk, bool prefer_idle,
		unsigned long min_util, int start_cpu,
		int best_idle, int best_active, int target),

	TP_ARGS(tsk, prefer_idle, min_util, start_cpu,
		best_idle, best_active, target),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( unsigned long,	min_util	)
		__field( bool,	prefer_idle		)
		__field( int,	start_cpu		)
		__field( int,	best_idle		)
		__field( int,	best_active		)
		__field( int,	target			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->min_util	= min_util;
		__entry->prefer_idle	= prefer_idle;
		__entry->start_cpu 	= start_cpu;
		__entry->best_idle	= best_idle;
		__entry->best_active	= best_active;
		__entry->target		= target;
	),

	TP_printk("pid=%d comm=%s prefer_idle=%d start_cpu=%d "
		  "best_idle=%d best_active=%d target=%d",
		__entry->pid, __entry->comm,
		__entry->prefer_idle, __entry->start_cpu,
		__entry->best_idle, __entry->best_active,
		__entry->target)
);

/*
 * Tracepoint for tasks' estimated utilization.
 */
TRACE_EVENT(sched_util_est_task,

	TP_PROTO(struct task_struct *tsk, struct sched_avg *avg),

	TP_ARGS(tsk, avg),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN		)
		__field( pid_t,		pid			)
		__field( int,		cpu			)
		__field( unsigned int,	util_avg		)
		__field( unsigned int,	est_enqueued		)
		__field( unsigned int,	est_ewma		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid			= tsk->pid;
		__entry->cpu                    = task_cpu(tsk);
		__entry->util_avg               = avg->util_avg;
		__entry->est_enqueued           = avg->util_est.enqueued;
		__entry->est_ewma               = avg->util_est.ewma;
	),

	TP_printk("comm=%s pid=%d cpu=%d util_avg=%u util_est_ewma=%u util_est_enqueued=%u",
		  __entry->comm,
		  __entry->pid,
		  __entry->cpu,
		  __entry->util_avg,
		  __entry->est_ewma,
		  __entry->est_enqueued)
);

/*
 * Tracepoint for root cfs_rq's estimated utilization.
 */
TRACE_EVENT(sched_util_est_cpu,

	TP_PROTO(int cpu, struct cfs_rq *cfs_rq),

	TP_ARGS(cpu, cfs_rq),

	TP_STRUCT__entry(
		__field( int,		cpu			)
		__field( unsigned int,	util_avg		)
		__field( unsigned int,	util_est_enqueued	)
	),

	TP_fast_assign(
		__entry->cpu			= cpu;
		__entry->util_avg		= cfs_rq->avg.util_avg;
		__entry->util_est_enqueued	= cfs_rq->avg.util_est.enqueued;
	),

	TP_printk("cpu=%d util_avg=%u util_est_enqueued=%u",
		  __entry->cpu,
		  __entry->util_avg,
		  __entry->util_est_enqueued)
);

#ifdef CONFIG_SCHED_WALT
struct rq;

TRACE_EVENT(walt_update_task_ravg,

	TP_PROTO(struct task_struct *p, struct rq *rq, int evt,
						u64 wallclock, u64 irqtime),

	TP_ARGS(p, rq, evt, wallclock, irqtime),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	pid_t,	cur_pid			)
		__field(	u64,	wallclock		)
		__field(	u64,	mark_start		)
		__field(	u64,	delta_m			)
		__field(	u64,	win_start		)
		__field(	u64,	delta			)
		__field(	u64,	irqtime			)
		__array(    char,   evt, 16			)
		__field(unsigned int,	demand			)
		__field(unsigned int,	sum			)
		__field(	 int,	cpu			)
		__field(	u64,	cs			)
		__field(	u64,	ps			)
		__field(	u32,	curr_window		)
		__field(	u32,	prev_window		)
		__field(	u64,	nt_cs			)
		__field(	u64,	nt_ps			)
		__field(	u32,	active_windows		)
	),

	TP_fast_assign(
			static const char* walt_event_names[] =
			{
				"PUT_PREV_TASK",
				"PICK_NEXT_TASK",
				"TASK_WAKE",
				"TASK_MIGRATE",
				"TASK_UPDATE",
				"IRQ_UPDATE"
			};
		__entry->wallclock      = wallclock;
		__entry->win_start      = rq->window_start;
		__entry->delta          = (wallclock - rq->window_start);
		strcpy(__entry->evt, walt_event_names[evt]);
		__entry->cpu            = rq->cpu;
		__entry->cur_pid        = rq->curr->pid;
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->mark_start     = p->ravg.mark_start;
		__entry->delta_m        = (wallclock - p->ravg.mark_start);
		__entry->demand         = p->ravg.demand;
		__entry->sum            = p->ravg.sum;
		__entry->irqtime        = irqtime;
		__entry->cs             = rq->curr_runnable_sum;
		__entry->ps             = rq->prev_runnable_sum;
		__entry->curr_window	= p->ravg.curr_window;
		__entry->prev_window	= p->ravg.prev_window;
		__entry->nt_cs		= rq->nt_curr_runnable_sum;
		__entry->nt_ps		= rq->nt_prev_runnable_sum;
		__entry->active_windows	= p->ravg.active_windows;
	),

	TP_printk("wallclock=%llu window_start=%llu delta=%llu event=%s cpu=%d cur_pid=%d pid=%d comm=%s"
		" mark_start=%llu delta=%llu demand=%u sum=%u irqtime=%llu"
		" curr_runnable_sum=%llu prev_runnable_sum=%llu cur_window=%u"
		" prev_window=%u nt_curr_runnable_sum=%llu nt_prev_runnable_sum=%llu active_windows=%u",
		__entry->wallclock, __entry->win_start, __entry->delta,
		__entry->evt, __entry->cpu, __entry->cur_pid,
		__entry->pid, __entry->comm, __entry->mark_start,
		__entry->delta_m, __entry->demand,
		__entry->sum, __entry->irqtime,
		__entry->cs, __entry->ps,
		__entry->curr_window, __entry->prev_window,
		__entry->nt_cs, __entry->nt_ps,
		__entry->active_windows
		)
);

TRACE_EVENT(walt_update_history,

	TP_PROTO(struct rq *rq, struct task_struct *p, u32 runtime, int samples,
			int evt),

	TP_ARGS(rq, p, runtime, samples, evt),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(unsigned int,	runtime			)
		__field(	 int,	samples			)
		__field(	 int,	evt			)
		__field(	 u64,	demand			)
		__field(unsigned int,	walt_avg		)
		__field(unsigned int,	pelt_avg		)
		__array(	 u32,	hist, RAVG_HIST_SIZE_MAX)
		__field(	 int,	cpu			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->runtime        = runtime;
		__entry->samples        = samples;
		__entry->evt            = evt;
		__entry->demand         = p->ravg.demand;
		walt_util(__entry->walt_avg,__entry->demand);
		__entry->pelt_avg	= p->se.avg.util_avg;
		memcpy(__entry->hist, p->ravg.sum_history,
					RAVG_HIST_SIZE_MAX * sizeof(u32));
		__entry->cpu            = rq->cpu;
	),

	TP_printk("pid=%d comm=%s runtime=%u samples=%d event=%d demand=%llu ravg_window=%u"
		" walt=%u pelt=%u hist0=%u hist1=%u hist2=%u hist3=%u hist4=%u cpu=%d",
		__entry->pid, __entry->comm,
		__entry->runtime, __entry->samples, __entry->evt,
		__entry->demand,
		walt_ravg_window,
		__entry->walt_avg,
		__entry->pelt_avg,
		__entry->hist[0], __entry->hist[1],
		__entry->hist[2], __entry->hist[3],
		__entry->hist[4], __entry->cpu)
);

TRACE_EVENT(walt_migration_update_sum,

	TP_PROTO(struct rq *rq, struct task_struct *p),

	TP_ARGS(rq, p),

	TP_STRUCT__entry(
		__field(int,		cpu			)
		__field(int,		pid			)
		__field(	u64,	cs			)
		__field(	u64,	ps			)
		__field(	s64,	nt_cs			)
		__field(	s64,	nt_ps			)
	),

	TP_fast_assign(
		__entry->cpu		= cpu_of(rq);
		__entry->cs		= rq->curr_runnable_sum;
		__entry->ps		= rq->prev_runnable_sum;
		__entry->nt_cs		= (s64)rq->nt_curr_runnable_sum;
		__entry->nt_ps		= (s64)rq->nt_prev_runnable_sum;
		__entry->pid		= p->pid;
	),

	TP_printk("cpu=%d curr_runnable_sum=%llu prev_runnable_sum=%llu nt_curr_runnable_sum=%lld nt_prev_runnable_sum=%lld pid=%d",
		  __entry->cpu, __entry->cs, __entry->ps,
		  __entry->nt_cs, __entry->nt_ps, __entry->pid)
);
#endif /* CONFIG_SCHED_WALT */
#endif /* CONFIG_SMP */

#ifdef CONFIG_UCLAMP_TASK

struct rq;

TRACE_EVENT(schedutil_uclamp_util,

	TP_PROTO(int cpu, unsigned long util),

	TP_ARGS(cpu, util),

	TP_STRUCT__entry(
		__field(int,		cpu)
		__field(unsigned long,	util)
		__field(unsigned int,	util_min)
		__field(unsigned int,	util_max)
	),

	TP_fast_assign(
		__entry->cpu			= cpu;
		__entry->util		= util;
		__entry->util_min	= uclamp_value(cpu, UCLAMP_MIN);
		__entry->util_max	= uclamp_value(cpu, UCLAMP_MAX);
	),

	TP_printk("cpu=%d util=%lu util_min=%u util_max=%u",
		  __entry->cpu,
		  __entry->util,
		  __entry->util_min,
		  __entry->util_max)
);

TRACE_EVENT(uclamp_cpu_get_id,

	TP_PROTO(struct task_struct *p, struct rq *rq, unsigned int clamp_id),

	TP_ARGS(p, rq, clamp_id),

	TP_STRUCT__entry(
		__field(int,		cpu)
		__field(int,	pid)
		__field(unsigned int,	clamp_id)
		__field(unsigned int,	task_uclamp_eff)
		__field(unsigned int,	rq_uclamp)
	),

	TP_fast_assign(
		__entry->cpu		= cpu_of(rq);
		__entry->pid		= p->pid;
		__entry->clamp_id	= clamp_id;
		__entry->task_uclamp_eff = p->uclamp[clamp_id].effective.value;
		__entry->rq_uclamp	= rq->uclamp.value[clamp_id];
	),

	TP_printk("cpu=%d pid=%d clamp_id=%u task_uclamp_eff=%u rq_uclamp=%u",
		  __entry->cpu,
		  __entry->pid,
		  __entry->clamp_id,
		  __entry->task_uclamp_eff,
		  __entry->rq_uclamp)
);

TRACE_EVENT(uclamp_cpu_put_id,

	TP_PROTO(struct task_struct *p, struct rq *rq, unsigned int clamp_id,
		unsigned int clamp_value),

	TP_ARGS(p, rq, clamp_id, clamp_value),

	TP_STRUCT__entry(
		__field(int,		cpu)
		__field(int,	pid)
		__field(unsigned int,	clamp_id)
		__field(unsigned int,	task_uclamp_eff)
		__field(unsigned int,	rq_uclamp)
	),

	TP_fast_assign(
		__entry->cpu		= cpu_of(rq);
		__entry->pid		= p->pid;
		__entry->clamp_id	= clamp_id;
		__entry->task_uclamp_eff	= clamp_value;
		__entry->rq_uclamp	= rq->uclamp.value[clamp_id];
	),

	TP_printk("cpu=%d pid=%d clamp_id=%u task_uclamp_eff=%u rq_uclamp=%u",
		  __entry->cpu,
		  __entry->pid,
		  __entry->clamp_id,
		  __entry->task_uclamp_eff,
		  __entry->rq_uclamp)
);

TRACE_EVENT_CONDITION(uclamp_util_se,

	TP_PROTO(bool is_task, struct task_struct *p, struct rq *rq),

	TP_ARGS(is_task, p, rq),

	TP_CONDITION(is_task),

	TP_STRUCT__entry(
		__field(pid_t,	pid)
		__array(char,	comm,   TASK_COMM_LEN)
		__field(int,	cpu)
		__field(unsigned int,	active)
		__field(unsigned long,	util_avg)
		__field(unsigned long,	uclamp_avg)
		__field(unsigned long,	uclamp_min)
		__field(unsigned long,	uclamp_max)
		__field(unsigned long,	uclamp_min_eff)
		__field(unsigned long,	uclamp_max_eff)
	),

	TP_fast_assign(
		__entry->pid            = p->pid;
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->cpu            = rq->cpu;
		__entry->active         = p->uclamp[UCLAMP_MIN].active;
		__entry->util_avg       = p->se.avg.util_avg;
		__entry->uclamp_avg     = uclamp_util(rq, p->se.avg.util_avg);
		__entry->uclamp_min = p->uclamp[UCLAMP_MIN].value;
		__entry->uclamp_max = p->uclamp[UCLAMP_MAX].value;
		__entry->uclamp_min_eff = p->uclamp[UCLAMP_MIN].effective.value;
		__entry->uclamp_max_eff = p->uclamp[UCLAMP_MAX].effective.value;
		),

	TP_printk("pid=%d comm=%s cpu=%d active=%u util_avg=%lu uclamp_avg=%lu uclamp_min=%lu uclamp_max=%lu uclamp_min_eff=%lu uclamp_max_eff=%lu",
		  __entry->pid, __entry->comm, __entry->cpu,
		  __entry->active, __entry->util_avg, __entry->uclamp_avg,
		  __entry->uclamp_min, __entry->uclamp_max,
		  __entry->uclamp_min_eff, __entry->uclamp_max_eff)
);

TRACE_EVENT_CONDITION(uclamp_util_cfs,

	TP_PROTO(bool is_root, int cpu, struct cfs_rq *cfs_rq),

	TP_ARGS(is_root, cpu, cfs_rq),

	TP_CONDITION(is_root),

	TP_STRUCT__entry(
		__field(int,	cpu)
		__field(unsigned long,	util_avg)
		__field(unsigned long,	uclamp_avg)
		__field(unsigned long,	uclamp_min)
		__field(unsigned long,	uclamp_max)
	),

	TP_fast_assign(
		__entry->cpu            = cpu;
		__entry->util_avg       = cfs_rq->avg.util_avg;
		__entry->uclamp_avg     = uclamp_util(cpu_rq(cpu),
						cfs_rq->avg.util_avg);
		__entry->uclamp_min     = cpu_rq(cpu)->uclamp.value[UCLAMP_MIN];
		__entry->uclamp_max     = cpu_rq(cpu)->uclamp.value[UCLAMP_MAX];
		),

	TP_printk("cpu=%d util_avg=%lu uclamp_avg=%lu uclamp_min=%lu uclamp_max=%lu",
		  __entry->cpu, __entry->util_avg, __entry->uclamp_avg,
		  __entry->uclamp_min, __entry->uclamp_max)
);
#else
#define trace_uclamp_util_dvfs(cpu, util) while (false) {}
#define trace_uclamp_util_se(is_task, p, rq) while (false) {}
#define trace_uclamp_util_cfs(is_root, cpu, cfs_rq) while (false) {}
#endif /* CONFIG_UCLAMP_TASK */

/*
 * Tracepoint for walt debug info.
 */
TRACE_EVENT(sched_ctl_walt,
		TP_PROTO(unsigned int user, int walted),
		TP_ARGS(user, walted),
		TP_STRUCT__entry(
			__field(unsigned int, user)
			__field(int, walted)
			),
		TP_fast_assign(
			__entry->user		= user;
			__entry->walted		= walted;
			),
		TP_printk("user_mask=0x%x walted=%d",
			__entry->user,
			__entry->walted
		)
);

/*
 * Tracepoint for average heavy task calculation.
 */
TRACE_EVENT(sched_heavy_task,
		TP_PROTO(const char *s),
		TP_ARGS(s),
		TP_STRUCT__entry(
			__string(s, s)
			),
		TP_fast_assign(
			__assign_str(s, s);
			),
		TP_printk("%s", __get_str(s))
);

TRACE_EVENT(sched_avg_heavy_task,

	TP_PROTO(int last_poll1, int last_poll2,
		int avg, int cluster_id, int max),

	TP_ARGS(last_poll1, last_poll2, avg, cluster_id, max),

	TP_STRUCT__entry(
		__field(int, last_poll1)
		__field(int, last_poll2)
		__field(int, avg)
		__field(int, cid)
		__field(int, max)
	),

	TP_fast_assign(
		__entry->last_poll1 = last_poll1;
		__entry->last_poll2 = last_poll2;
		__entry->avg = avg;
		__entry->cid = cluster_id;
		__entry->max = max;
	),

	TP_printk("last_poll1=%d last_poll2=%d, avg=%d, max:%d, cid:%d",
		__entry->last_poll1,
		__entry->last_poll2,
		__entry->avg,
		__entry->max,
		__entry->cid)
);

TRACE_EVENT(sched_avg_heavy_nr,
	TP_PROTO(int invoker, int nr_heavy,
		long long int diff, int ack_cap, int cpu),

	TP_ARGS(invoker, nr_heavy, diff, ack_cap, cpu),

	TP_STRUCT__entry(
		__field(int, invoker)
		__field(int, nr_heavy)
		__field(long long int, diff)
		__field(int, ack_cap)
		__field(int, cpu)
	),

	TP_fast_assign(
		__entry->invoker = invoker;
		__entry->nr_heavy = nr_heavy;
		__entry->diff = diff;
		__entry->ack_cap = ack_cap;
		__entry->cpu = cpu;
	),

	TP_printk("invoker=%d nr_heavy=%d time diff:%lld ack_cap:%d cpu:%d",
		__entry->invoker,
		__entry->nr_heavy, __entry->diff, __entry->ack_cap, __entry->cpu
	)
);

TRACE_EVENT(sched_avg_heavy_time,
	TP_PROTO(long long int time_period,
		long long int last_get_heavy_time, int cid),

	TP_ARGS(time_period, last_get_heavy_time, cid),

	TP_STRUCT__entry(
		__field(long long int, time_period)
		__field(long long int, last_get_heavy_time)
		__field(int, cid)
	),

	TP_fast_assign(
		__entry->time_period = time_period;
		__entry->last_get_heavy_time = last_get_heavy_time;
		__entry->cid = cid;
	),

	TP_printk("time_period:%lld last_get_heavy_time:%lld cid:%d",
		__entry->time_period, __entry->last_get_heavy_time, __entry->cid
	)
)

TRACE_EVENT(sched_avg_heavy_task_load,
	TP_PROTO(struct task_struct *t),

	TP_ARGS(t),

	TP_STRUCT__entry(
		__array(char,   comm,   TASK_COMM_LEN)
		__field(pid_t,  pid)
		__field(long long int,  load)
	),

	TP_fast_assign(
		memcpy(__entry->comm, t->comm, TASK_COMM_LEN);
		__entry->pid = t->pid;
		__entry->load = t->se.avg.load_avg;
	),

	TP_printk("heavy_task_detect comm:%s pid:%d load:%lld",
		__entry->comm, __entry->pid, __entry->load
	)
)

/**
 * sched_isolate - called when cores are isolated/unisolated
 *
 * @acutal_mask: mask of cores actually isolated/unisolated
 * @req_mask: mask of cores requested isolated/unisolated
 * @online_mask: cpu online mask
 * @time: amount of time in us it took to isolate/unisolate
 * @isolate: 1 if isolating, 0 if unisolating
 *
 */
TRACE_EVENT(sched_isolate,

	TP_PROTO(unsigned int requested_cpu, unsigned int isolated_cpus,
		 u64 start_time, unsigned char isolate),

	TP_ARGS(requested_cpu, isolated_cpus, start_time, isolate),

	TP_STRUCT__entry(
		__field(u32, requested_cpu)
		__field(u32, isolated_cpus)
		__field(u32, time)
		__field(unsigned char, isolate)
	),

	TP_fast_assign(
		__entry->requested_cpu = requested_cpu;
		__entry->isolated_cpus = isolated_cpus;
		__entry->time = div64_u64(sched_clock() - start_time, 1000);
		__entry->isolate = isolate;
	),

	TP_printk("iso cpu=%u cpus=0x%x time=%u us isolated=%d",
		  __entry->requested_cpu, __entry->isolated_cpus,
		  __entry->time, __entry->isolate)
);


#include "sched_enhance.h"

#endif /* _TRACE_SCHED_H */
/* This part must be outside protection */
#include <trace/define_trace.h>
