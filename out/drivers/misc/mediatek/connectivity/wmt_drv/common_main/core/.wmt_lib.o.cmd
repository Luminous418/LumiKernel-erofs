cmd_drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/wmt_lib.o := /home/luminous418/zyc-clang/bin/clang -Wp,-MD,drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/.wmt_lib.o.d -nostdinc -isystem /home/luminous418/zyc-clang/lib/clang/14.0.6/include -I../arch/arm64/include -I./arch/arm64/include/generated  -I../include -I../drivers/misc/mediatek/include -I./include -I../arch/arm64/include/uapi -I./arch/arm64/include/generated/uapi -I../include/uapi -I./include/generated/uapi -include ../include/linux/kconfig.h  -I../drivers/misc/mediatek/connectivity/wmt_drv -Idrivers/misc/mediatek/connectivity/wmt_drv -D__KERNEL__ -Qunused-arguments -mlittle-endian -DKASAN_SHADOW_SCALE_SHIFT=3 -O3 -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 --target=aarch64-linux-gnu --prefix=/home/luminous418/zyc-clang/bin/aarch64-linux-gnu- --gcc-toolchain=/home/luminous418/zyc-clang -no-integrated-as -Werror=unknown-warning-option -fuse-ld=lld -fno-PIE -mno-implicit-float -DCONFIG_AS_LSE=1 -fno-asynchronous-unwind-tables -fno-pic -Wno-asm-operand-widths -DKASAN_SHADOW_SCALE_SHIFT=3 -fno-delete-null-pointer-checks -Wno-frame-address -Wno-int-in-bool-context -Wno-address-of-packed-member -O3 --param=allow-store-data-races=0 -mcpu=cortex-a55 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=2800 -fstack-protector-strong -Wno-format-invalid-specifier -Wno-gnu -Wno-duplicate-decl-specifier -Wno-tautological-compare -mno-global-merge -Wno-unused-but-set-variable -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -Wno-array-bounds -fno-strict-overflow -fno-merge-all-constants -fno-stack-check -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -fmacro-prefix-map=../= -Wno-initializer-overrides -Wno-unused-value -Wno-format -Wno-sign-compare -Wno-format-zero-length -Wno-uninitialized -Wno-pointer-to-enum-cast -Wno-unaligned-access -Wno-enum-compare-conditional -Wno-enum-enum-conversion -w -pipe -O3 -Werror  -I../drivers/misc/mediatek/include  -I../drivers/misc/mediatek/include/mt-plat/mt6768/include/  -I../drivers/misc/mediatek/include/mt-plat/  -I../drivers/mmc/host/mediatek/mt6768  -I../drivers/misc/mediatek/base/power/include  -I../drivers/misc/mediatek/clkbuf/src  -I../drivers/misc/mediatek/base/power/include/clkbuf_v1/mt6768 -Werror  -I../drivers/misc/mediatek/include/mt-plat/mt6768/include -Werror  -I../drivers/misc/mediatek/include/mt-plat  -I../drivers/mmc/core  -I../drivers/misc/mediatek/eccci/mt6768  -I../drivers/misc/mediatek/eccci/  -I../drivers/clk/mediatek/  -I../drivers/pinctrl/mediatek/ -Werror  -I../drivers/misc/mediatek/pmic/include -D MTK_WCN_REMOVE_KERNEL_MODULE -D MTK_WCN_BUILT_IN_DRIVER -D CONFIG_MTK_WCN_ARM64 -D WMT_IDC_SUPPORT=1 -D MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT  -I../drivers/misc/mediatek/include  -I../drivers/misc/mediatek/include/mt-plat/mt6768/include  -I../drivers/misc/mediatek/include/mt-plat/mt6768/include/mach  -I../drivers/misc/mediatek/include/mt-plat  -I../drivers/misc/mediatek/base/power/mt6768  -I../drivers/misc/mediatek/base/power/include  -I../drivers/misc/mediatek/base/power/include/clkbuf_v1  -I../drivers/misc/mediatek/base/power/include/clkbuf_v1/mt6768  -I../drivers/misc/mediatek/btif/common/inc  -I../drivers/misc/mediatek/eccci  -I../drivers/misc/mediatek/eccci/mt6768  -I../drivers/misc/mediatek/eemcs  -I../drivers/misc/mediatek/conn_md/include  -I../drivers/misc/mediatek/mach/mt6768/include/mach  -I../drivers/misc/mediatek/emi/submodule  -I../drivers/misc/mediatek/emi/mt6768  -I../drivers/mmc/core  -I../drivers/misc/mediatek/connectivity/common  -I../drivers/misc/mediatek/include/mt-plat -Werror -D CONSYS_PMIC_CTRL_6635=1  -I../arch/arm/mach-mt6768//dct/dct -DWMT_PLAT_ALPS=1 -D MTK_WCN_SOC_CHIP_SUPPORT  -I../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include  -I../drivers/misc/mediatek/connectivity/wmt_drv/common_detect/drv_init/inc  -I../drivers/misc/mediatek/connectivity/wmt_drv/common_detect  -I../drivers/misc/mediatek/connectivity/wmt_drv/debug_utility -D MTK_WCN_WLAN_GEN2  -I../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include  -I../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/pri/include  -I../drivers/misc/mediatek/connectivity/wmt_drv/common_main/platform/include  -I../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include  -I../drivers/misc/mediatek/connectivity/wmt_drv/common_main/include -D WMT_PLAT_ALPS=1 -D WMT_UART_RX_MODE_WORK=0 -D WMT_SDIO_MODE=1 -D WMT_CREATE_NODE_DYNAMIC=1 -D WMT_DBG_SUPPORT=1 -D WMT_DEVAPC_DBG_SUPPORT=1 -D CFG_WMT_STEP    -DKBUILD_BASENAME='"wmt_lib"'  -DKBUILD_MODNAME='"wmt_drv"' -c -o drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/.tmp_wmt_lib.o ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/wmt_lib.c

source_drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/wmt_lib.o := ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/wmt_lib.c

deps_drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/wmt_lib.o := \
    $(wildcard include/config/mtk/combo/chip/deep/sleep/support.h) \
    $(wildcard include/config/mtk/eng/build.h) \
    $(wildcard include/config/mt/eng/build.h) \
  ../include/linux/compiler_types.h \
    $(wildcard include/config/have/arch/compiler/h.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  ../include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
    $(wildcard include/config/retpoline.h) \
    $(wildcard include/config/arm64.h) \
    $(wildcard include/config/gcov/kernel.h) \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  ../include/linux/compiler-clang.h \
    $(wildcard include/config/lto/clang.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/osal_typedef.h \
    $(wildcard include/config/mtk/aee/aed.h) \
  include/generated/uapi/linux/version.h \
  ../include/linux/init.h \
    $(wildcard include/config/strict/kernel/rwx.h) \
    $(wildcard include/config/strict/module/rwx.h) \
  ../include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/stack/validation.h) \
    $(wildcard include/config/kasan.h) \
  ../include/uapi/linux/types.h \
  arch/arm64/include/generated/uapi/asm/types.h \
  ../include/uapi/asm-generic/types.h \
  ../include/asm-generic/int-ll64.h \
  ../include/uapi/asm-generic/int-ll64.h \
  ../arch/arm64/include/uapi/asm/bitsperlong.h \
  ../include/asm-generic/bitsperlong.h \
    $(wildcard include/config/64bit.h) \
  ../include/uapi/asm-generic/bitsperlong.h \
  ../include/uapi/linux/posix_types.h \
  ../include/linux/stddef.h \
  ../include/uapi/linux/stddef.h \
  ../arch/arm64/include/uapi/asm/posix_types.h \
  ../include/uapi/asm-generic/posix_types.h \
  ../arch/arm64/include/asm/barrier.h \
  ../include/asm-generic/barrier.h \
    $(wildcard include/config/smp.h) \
  ../include/linux/kasan-checks.h \
  ../include/linux/types.h \
    $(wildcard include/config/have/uid16.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
  ../include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/modules/tree/lookup.h) \
    $(wildcard include/config/livepatch.h) \
    $(wildcard include/config/cfi/clang.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/module/sig.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/constructors.h) \
  ../include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  ../include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
    $(wildcard include/config/page/poisoning/zero.h) \
  ../include/linux/const.h \
  ../include/uapi/linux/const.h \
  ../include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/arch/has/refcount.h) \
    $(wildcard include/config/panic/timeout.h) \
    $(wildcard include/config/disable/trace/printk.h) \
  /home/luminous418/zyc-clang/lib/clang/14.0.6/include/stdarg.h \
  ../include/linux/linkage.h \
    $(wildcard include/config/rustuh/rkp.h) \
    $(wildcard include/config/uh/rkp.h) \
    $(wildcard include/config/rustuh/kdp.h) \
    $(wildcard include/config/kdp/cred.h) \
  ../include/linux/stringify.h \
  ../include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/module/rel/crcs.h) \
    $(wildcard include/config/trim/unused/ksyms.h) \
    $(wildcard include/config/sec/kunit.h) \
    $(wildcard include/config/kunit.h) \
  ../arch/arm64/include/asm/linkage.h \
  ../include/linux/bitops.h \
  ../include/linux/bits.h \
  ../arch/arm64/include/asm/bitops.h \
  ../include/asm-generic/bitops/builtin-__ffs.h \
  ../include/asm-generic/bitops/builtin-ffs.h \
  ../include/asm-generic/bitops/builtin-__fls.h \
  ../include/asm-generic/bitops/builtin-fls.h \
  ../include/asm-generic/bitops/ffz.h \
  ../include/asm-generic/bitops/fls64.h \
  ../include/asm-generic/bitops/find.h \
    $(wildcard include/config/generic/find/first/bit.h) \
  ../include/asm-generic/bitops/sched.h \
  ../include/asm-generic/bitops/hweight.h \
  ../include/asm-generic/bitops/arch_hweight.h \
  ../include/asm-generic/bitops/const_hweight.h \
  ../include/asm-generic/bitops/lock.h \
  ../include/asm-generic/bitops/non-atomic.h \
  ../include/asm-generic/bitops/le.h \
  ../arch/arm64/include/uapi/asm/byteorder.h \
  ../include/linux/byteorder/little_endian.h \
    $(wildcard include/config/cpu/big/endian.h) \
  ../include/uapi/linux/byteorder/little_endian.h \
  ../include/linux/swab.h \
  ../include/uapi/linux/swab.h \
  arch/arm64/include/generated/uapi/asm/swab.h \
  ../include/uapi/asm-generic/swab.h \
  ../include/linux/byteorder/generic.h \
  ../include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  ../include/linux/typecheck.h \
  ../include/linux/printk.h \
    $(wildcard include/config/mtk/aee/feature.h) \
    $(wildcard include/config/printk/mt/prefix.h) \
    $(wildcard include/config/sec/debug/auto/comment.h) \
    $(wildcard include/config/message/loglevel/default.h) \
    $(wildcard include/config/console/loglevel/default.h) \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk/nmi.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  ../include/linux/kern_levels.h \
  ../include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  ../include/uapi/linux/kernel.h \
  ../include/uapi/linux/sysinfo.h \
  ../arch/arm64/include/asm/cache.h \
    $(wildcard include/config/kasan/sw/tags.h) \
  ../arch/arm64/include/asm/cputype.h \
  ../arch/arm64/include/asm/sysreg.h \
    $(wildcard include/config/broken/gas/inst.h) \
    $(wildcard include/config/arm64/4k/pages.h) \
    $(wildcard include/config/arm64/16k/pages.h) \
    $(wildcard include/config/arm64/64k/pages.h) \
  ../arch/arm64/include/asm/compiler.h \
  ../include/linux/build_bug.h \
  ../include/linux/dynamic_debug.h \
    $(wildcard include/config/jump/label.h) \
  ../include/linux/stat.h \
  ../arch/arm64/include/asm/stat.h \
    $(wildcard include/config/compat.h) \
  ../arch/arm64/include/uapi/asm/stat.h \
  ../include/uapi/asm-generic/stat.h \
  ../arch/arm64/include/asm/compat.h \
  ../include/linux/sched.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/virt/cpu/accounting/native.h) \
    $(wildcard include/config/sched/info.h) \
    $(wildcard include/config/sched/hmp.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/sched/hmp/prio/filter.h) \
    $(wildcard include/config/sched/bore.h) \
    $(wildcard include/config/fair/group/sched.h) \
    $(wildcard include/config/mtk/rt/throttle/mon.h) \
    $(wildcard include/config/sched/walt.h) \
    $(wildcard include/config/rt/group/sched.h) \
    $(wildcard include/config/uclamp/task.h) \
    $(wildcard include/config/uclamp/groups/count.h) \
    $(wildcard include/config/five.h) \
    $(wildcard include/config/thread/info/in/task.h) \
    $(wildcard include/config/mtk/sched/boost.h) \
    $(wildcard include/config/cgroup/sched.h) \
    $(wildcard include/config/preempt/notifiers.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/tasks/rcu.h) \
    $(wildcard include/config/psi.h) \
    $(wildcard include/config/memcg.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/compat/brk.h) \
    $(wildcard include/config/cgroups.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/arch/has/scaled/cputime.h) \
    $(wildcard include/config/cpu/freq/times.h) \
    $(wildcard include/config/virt/cpu/accounting/gen.h) \
    $(wildcard include/config/no/hz/full.h) \
    $(wildcard include/config/mtk/mlog.h) \
    $(wildcard include/config/swap.h) \
    $(wildcard include/config/posix/timers.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/detect/hung/task.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lockdep/crossrelease.h) \
    $(wildcard include/config/ubsan.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/intel/rdt.h) \
    $(wildcard include/config/futex.h) \
    $(wildcard include/config/perf/events.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/numa/balancing.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/fault/injection.h) \
    $(wildcard include/config/latencytop.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/kcov.h) \
    $(wildcard include/config/uprobes.h) \
    $(wildcard include/config/bcache.h) \
    $(wildcard include/config/sdp.h) \
    $(wildcard include/config/vmap/stack.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/mtk/task/turbo.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/sched/tune.h) \
  ../include/uapi/linux/sched.h \
  ../arch/arm64/include/asm/current.h \
  ../include/linux/pid.h \
  ../include/linux/rculist.h \
  ../include/linux/rcupdate.h \
    $(wildcard include/config/rcu/stall/common.h) \
    $(wildcard include/config/rcu/nocb/cpu.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/tiny/rcu.h) \
    $(wildcard include/config/debug/objects/rcu/head.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/prove/rcu.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
    $(wildcard include/config/rcu/boost.h) \
    $(wildcard include/config/arch/weak/release/acquire.h) \
  ../include/linux/atomic.h \
    $(wildcard include/config/generic/atomic64.h) \
  ../arch/arm64/include/asm/atomic.h \
    $(wildcard include/config/arm64/lse/atomics.h) \
    $(wildcard include/config/as/lse.h) \
  ../arch/arm64/include/asm/lse.h \
  ../arch/arm64/include/asm/atomic_ll_sc.h \
  ../arch/arm64/include/asm/cmpxchg.h \
  ../include/linux/bug.h \
    $(wildcard include/config/bug/on/data/corruption.h) \
  ../arch/arm64/include/asm/bug.h \
  ../arch/arm64/include/asm/asm-bug.h \
    $(wildcard include/config/debug/bugverbose.h) \
  ../arch/arm64/include/asm/brk-imm.h \
  ../include/asm-generic/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  ../include/asm-generic/atomic-long.h \
  ../include/linux/irqflags.h \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  ../arch/arm64/include/asm/irqflags.h \
  ../arch/arm64/include/asm/ptrace.h \
  ../arch/arm64/include/uapi/asm/ptrace.h \
  ../arch/arm64/include/asm/hwcap.h \
  ../arch/arm64/include/uapi/asm/hwcap.h \
  ../include/asm-generic/ptrace.h \
  ../include/linux/preempt.h \
    $(wildcard include/config/preempt/count.h) \
  arch/arm64/include/generated/asm/preempt.h \
  ../include/asm-generic/preempt.h \
  ../include/linux/thread_info.h \
    $(wildcard include/config/have/arch/within/stack/frames.h) \
    $(wildcard include/config/hardened/usercopy.h) \
  ../include/linux/restart_block.h \
  ../include/linux/errno.h \
  ../include/uapi/linux/errno.h \
  arch/arm64/include/generated/uapi/asm/errno.h \
  ../include/uapi/asm-generic/errno.h \
  ../include/uapi/asm-generic/errno-base.h \
  ../arch/arm64/include/asm/thread_info.h \
    $(wildcard include/config/arm64/sw/ttbr0/pan.h) \
    $(wildcard include/config/shadow/call/stack.h) \
  ../arch/arm64/include/asm/memory.h \
    $(wildcard include/config/arm64/va/bits.h) \
    $(wildcard include/config/debug/align/rodata.h) \
    $(wildcard include/config/blk/dev/initrd.h) \
    $(wildcard include/config/debug/virtual.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
  ../arch/arm64/include/asm/page-def.h \
    $(wildcard include/config/arm64/page/shift.h) \
    $(wildcard include/config/arm64/cont/shift.h) \
  arch/arm64/include/generated/asm/sizes.h \
  ../include/asm-generic/sizes.h \
  ../include/linux/sizes.h \
  ../include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/vm/pgflags.h) \
  ../include/asm-generic/memory_model.h \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem.h) \
  ../include/linux/pfn.h \
  ../arch/arm64/include/asm/stack_pointer.h \
  ../include/linux/bottom_half.h \
  ../include/linux/lockdep.h \
    $(wildcard include/config/lock/stat.h) \
  ../arch/arm64/include/asm/processor.h \
    $(wildcard include/config/have/hw/breakpoint.h) \
    $(wildcard include/config/arm64/tagged/addr/abi.h) \
  ../include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
    $(wildcard include/config/fortify/source.h) \
  ../include/uapi/linux/string.h \
  ../arch/arm64/include/asm/string.h \
    $(wildcard include/config/arch/has/uaccess/flushcache.h) \
  ../arch/arm64/include/asm/alternative.h \
    $(wildcard include/config/arm64/uao.h) \
    $(wildcard include/config/foo.h) \
  ../arch/arm64/include/asm/cpucaps.h \
  ../arch/arm64/include/asm/insn.h \
  ../arch/arm64/include/asm/cpufeature.h \
    $(wildcard include/config/arm64/ssbd.h) \
  ../include/linux/jump_label.h \
  ../arch/arm64/include/asm/fpsimd.h \
  ../arch/arm64/include/asm/hw_breakpoint.h \
  ../arch/arm64/include/asm/virt.h \
  ../arch/arm64/include/asm/sections.h \
  ../include/asm-generic/sections.h \
  ../arch/arm64/include/asm/pgtable-hwdef.h \
    $(wildcard include/config/pgtable/levels.h) \
  ../include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  ../include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  ../include/linux/bitmap.h \
  ../include/linux/rcutree.h \
  ../include/linux/wait.h \
  ../include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
  ../include/linux/spinlock_types.h \
  ../arch/arm64/include/asm/spinlock_types.h \
  ../include/linux/rwlock_types.h \
  ../arch/arm64/include/asm/spinlock.h \
  ../include/linux/rwlock.h \
  ../include/linux/spinlock_api_smp.h \
    $(wildcard include/config/inline/spin/lock.h) \
    $(wildcard include/config/inline/spin/lock/bh.h) \
    $(wildcard include/config/inline/spin/lock/irq.h) \
    $(wildcard include/config/inline/spin/lock/irqsave.h) \
    $(wildcard include/config/inline/spin/trylock.h) \
    $(wildcard include/config/inline/spin/trylock/bh.h) \
    $(wildcard include/config/uninline/spin/unlock.h) \
    $(wildcard include/config/inline/spin/unlock/bh.h) \
    $(wildcard include/config/inline/spin/unlock/irq.h) \
    $(wildcard include/config/inline/spin/unlock/irqrestore.h) \
  ../include/linux/rwlock_api_smp.h \
    $(wildcard include/config/inline/read/lock.h) \
    $(wildcard include/config/inline/write/lock.h) \
    $(wildcard include/config/inline/read/lock/bh.h) \
    $(wildcard include/config/inline/write/lock/bh.h) \
    $(wildcard include/config/inline/read/lock/irq.h) \
    $(wildcard include/config/inline/write/lock/irq.h) \
    $(wildcard include/config/inline/read/lock/irqsave.h) \
    $(wildcard include/config/inline/write/lock/irqsave.h) \
    $(wildcard include/config/inline/read/trylock.h) \
    $(wildcard include/config/inline/write/trylock.h) \
    $(wildcard include/config/inline/read/unlock.h) \
    $(wildcard include/config/inline/write/unlock.h) \
    $(wildcard include/config/inline/read/unlock/bh.h) \
    $(wildcard include/config/inline/write/unlock/bh.h) \
    $(wildcard include/config/inline/read/unlock/irq.h) \
    $(wildcard include/config/inline/write/unlock/irq.h) \
    $(wildcard include/config/inline/read/unlock/irqrestore.h) \
    $(wildcard include/config/inline/write/unlock/irqrestore.h) \
  ../include/uapi/linux/wait.h \
  ../include/linux/sem.h \
  ../include/linux/time64.h \
  ../include/uapi/linux/time.h \
  ../include/linux/math64.h \
    $(wildcard include/config/arch/supports/int128.h) \
  arch/arm64/include/generated/asm/div64.h \
  ../include/asm-generic/div64.h \
  ../include/uapi/linux/sem.h \
  ../include/linux/ipc.h \
  ../include/linux/uidgid.h \
    $(wildcard include/config/multiuser.h) \
    $(wildcard include/config/user/ns.h) \
  ../include/linux/highuid.h \
  ../include/linux/rhashtable.h \
  ../include/linux/err.h \
  ../include/linux/jhash.h \
  ../include/linux/unaligned/packed_struct.h \
  ../include/linux/list_nulls.h \
  ../include/linux/workqueue.h \
    $(wildcard include/config/debug/objects/work.h) \
    $(wildcard include/config/freezer.h) \
    $(wildcard include/config/wq/watchdog.h) \
  ../include/linux/timer.h \
    $(wildcard include/config/debug/objects/timers.h) \
    $(wildcard include/config/no/hz/common.h) \
  ../include/linux/ktime.h \
  ../include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  ../include/linux/seqlock.h \
  ../include/linux/jiffies.h \
  ../include/linux/timex.h \
  ../include/uapi/linux/timex.h \
  ../include/uapi/linux/param.h \
  ../arch/arm64/include/uapi/asm/param.h \
  ../include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  ../include/uapi/asm-generic/param.h \
  ../arch/arm64/include/asm/timex.h \
  ../arch/arm64/include/asm/arch_timer.h \
    $(wildcard include/config/arm/arch/timer/ool/workaround.h) \
  ../include/linux/smp.h \
    $(wildcard include/config/up/late/init.h) \
  ../include/linux/llist.h \
    $(wildcard include/config/arch/have/nmi/safe/cmpxchg.h) \
  ../arch/arm64/include/asm/smp.h \
    $(wildcard include/config/arm64/acpi/parking/protocol.h) \
  ../arch/arm64/include/asm/percpu.h \
  ../include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  ../include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
    $(wildcard include/config/mtk/sched/monitor.h) \
  ../include/clocksource/arm_arch_timer.h \
    $(wildcard include/config/arm/arch/timer.h) \
  ../include/linux/timecounter.h \
  ../include/asm-generic/timex.h \
  include/generated/timeconst.h \
  ../include/linux/timekeeping.h \
  ../include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/debug/objects/free.h) \
  ../include/linux/mutex.h \
    $(wildcard include/config/mutex/spin/on/owner.h) \
  ../include/linux/osq_lock.h \
  ../include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  ../include/uapi/linux/ipc.h \
  arch/arm64/include/generated/uapi/asm/ipcbuf.h \
  ../include/uapi/asm-generic/ipcbuf.h \
  ../include/linux/refcount.h \
    $(wildcard include/config/refcount/full.h) \
  arch/arm64/include/generated/uapi/asm/sembuf.h \
  ../include/uapi/asm-generic/sembuf.h \
  ../include/linux/shm.h \
  ../arch/arm64/include/asm/page.h \
    $(wildcard include/config/have/arch/pfn/valid.h) \
  ../include/linux/personality.h \
  ../include/uapi/linux/personality.h \
  ../arch/arm64/include/asm/pgtable-types.h \
  ../include/asm-generic/pgtable-nopud.h \
  ../include/asm-generic/pgtable-nop4d-hack.h \
  ../include/asm-generic/5level-fixup.h \
  ../include/asm-generic/getorder.h \
  ../include/uapi/linux/shm.h \
  ../include/uapi/asm-generic/hugetlb_encode.h \
  arch/arm64/include/generated/uapi/asm/shmbuf.h \
  ../include/uapi/asm-generic/shmbuf.h \
  ../arch/arm64/include/asm/shmparam.h \
  ../include/uapi/asm-generic/shmparam.h \
  ../include/linux/kcov.h \
  ../include/uapi/linux/kcov.h \
  ../include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  ../include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
    $(wildcard include/config/time/low/res.h) \
    $(wildcard include/config/timerfd.h) \
  ../include/linux/rbtree.h \
  ../include/linux/percpu.h \
    $(wildcard include/config/need/per/cpu/embed/first/chunk.h) \
    $(wildcard include/config/need/per/cpu/page/first/chunk.h) \
  ../include/linux/timerqueue.h \
  ../include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
    $(wildcard include/config/have/arch/seccomp/filter.h) \
    $(wildcard include/config/seccomp/filter.h) \
    $(wildcard include/config/checkpoint/restore.h) \
  ../include/uapi/linux/seccomp.h \
  ../arch/arm64/include/asm/seccomp.h \
  ../arch/arm64/include/asm/unistd.h \
  ../arch/arm64/include/uapi/asm/unistd.h \
  ../include/asm-generic/unistd.h \
  ../include/uapi/asm-generic/unistd.h \
  ../include/asm-generic/seccomp.h \
  ../include/uapi/linux/unistd.h \
  ../include/linux/nodemask.h \
    $(wildcard include/config/highmem.h) \
  ../include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  ../include/linux/resource.h \
  ../include/uapi/linux/resource.h \
  arch/arm64/include/generated/uapi/asm/resource.h \
  ../include/asm-generic/resource.h \
  ../include/uapi/asm-generic/resource.h \
  ../include/linux/latencytop.h \
  ../include/linux/sched/prio.h \
  ../include/linux/signal_types.h \
    $(wildcard include/config/old/sigaction.h) \
  ../include/uapi/linux/signal.h \
  ../arch/arm64/include/uapi/asm/signal.h \
  ../include/asm-generic/signal.h \
  ../include/uapi/asm-generic/signal.h \
  ../include/uapi/asm-generic/signal-defs.h \
  ../arch/arm64/include/uapi/asm/sigcontext.h \
  ../arch/arm64/include/uapi/asm/siginfo.h \
  ../include/uapi/asm-generic/siginfo.h \
  ../include/linux/mm_types_task.h \
    $(wildcard include/config/arch/want/batched/unmap/tlb/flush.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/arch/enable/split/pmd/ptlock.h) \
  ../include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  ../include/linux/sched/sched.h \
    $(wildcard include/config/mtk/sched/trace.h) \
    $(wildcard include/config/mtk/sched/debug.h) \
    $(wildcard include/config/mtk/sched/eas/power/support.h) \
    $(wildcard include/config/mach/mt6873.h) \
    $(wildcard include/config/mtk/sched/interop.h) \
  ../include/linux/sched/task_stack.h \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  ../include/uapi/linux/magic.h \
  ../include/uapi/linux/stat.h \
  ../include/linux/kmod.h \
  ../include/linux/umh.h \
  ../include/linux/gfp.h \
    $(wildcard include/config/dmauser/pages.h) \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/zone/device.h) \
    $(wildcard include/config/zone/movable/cma.h) \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/compaction.h) \
    $(wildcard include/config/memory/isolation.h) \
    $(wildcard include/config/cma.h) \
  ../include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/zsmalloc.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/page/extension.h) \
    $(wildcard include/config/no/bootmem.h) \
    $(wildcard include/config/deferred/struct/page/init.h) \
    $(wildcard include/config/transparent/hugepage.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/have/memoryless/nodes.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/have/memblock/node/map.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/memory/hotremove.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  ../include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  ../include/linux/page-flags-layout.h \
  include/generated/bounds.h \
  ../arch/arm64/include/asm/sparsemem.h \
  ../include/linux/memory_hotplug.h \
    $(wildcard include/config/arch/has/add/pages.h) \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/have/bootmem/info/node.h) \
  ../include/linux/notifier.h \
  ../include/linux/rwsem.h \
    $(wildcard include/config/rwsem/spin/on/owner.h) \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  arch/arm64/include/generated/asm/rwsem.h \
  ../include/asm-generic/rwsem.h \
  ../include/linux/srcu.h \
    $(wildcard include/config/tiny/srcu.h) \
    $(wildcard include/config/tree/srcu.h) \
    $(wildcard include/config/srcu.h) \
  ../include/linux/rcu_segcblist.h \
  ../include/linux/srcutree.h \
  ../include/linux/rcu_node_tree.h \
    $(wildcard include/config/rcu/fanout.h) \
    $(wildcard include/config/rcu/fanout/leaf.h) \
  ../include/linux/completion.h \
    $(wildcard include/config/lockdep/completions.h) \
  ../include/linux/topology.h \
    $(wildcard include/config/use/percpu/numa/node/id.h) \
    $(wildcard include/config/sched/smt.h) \
  ../arch/arm64/include/asm/topology.h \
  ../include/linux/arch_topology.h \
  ../include/asm-generic/topology.h \
  ../include/linux/sysctl.h \
    $(wildcard include/config/sysctl.h) \
  ../include/uapi/linux/sysctl.h \
  ../include/linux/elf.h \
  ../arch/arm64/include/asm/elf.h \
  arch/arm64/include/generated/asm/user.h \
  ../include/asm-generic/user.h \
  ../include/uapi/linux/elf.h \
  ../include/uapi/linux/elf-em.h \
  ../include/linux/kobject.h \
    $(wildcard include/config/uevent/helper.h) \
    $(wildcard include/config/debug/kobject/release.h) \
  ../include/linux/sysfs.h \
  ../include/linux/kernfs.h \
    $(wildcard include/config/kernfs.h) \
  ../include/linux/idr.h \
  ../include/linux/radix-tree.h \
    $(wildcard include/config/radix/tree/multiorder.h) \
  ../include/linux/kobject_ns.h \
  ../include/linux/kref.h \
  ../include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  ../include/linux/rbtree_latch.h \
  ../include/linux/cfi.h \
    $(wildcard include/config/cfi/clang/shadow.h) \
  ../arch/arm64/include/asm/module.h \
    $(wildcard include/config/arm64/module/plts.h) \
    $(wildcard include/config/dynamic/ftrace.h) \
    $(wildcard include/config/randomize/base.h) \
  ../include/asm-generic/module.h \
    $(wildcard include/config/have/mod/arch/specific.h) \
    $(wildcard include/config/modules/use/elf/rel.h) \
    $(wildcard include/config/modules/use/elf/rela.h) \
  ../include/linux/fs.h \
    $(wildcard include/config/fs/posix/acl.h) \
    $(wildcard include/config/cgroup/writeback.h) \
    $(wildcard include/config/ima.h) \
    $(wildcard include/config/fsnotify.h) \
    $(wildcard include/config/fs/encryption.h) \
    $(wildcard include/config/fs/verity.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/five/pa/feature.h) \
    $(wildcard include/config/proca.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/unicode.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/fs/dax.h) \
    $(wildcard include/config/mandatory/file/locking.h) \
    $(wildcard include/config/migration.h) \
  ../include/linux/wait_bit.h \
  ../include/linux/kdev_t.h \
  ../include/uapi/linux/kdev_t.h \
  ../include/linux/dcache.h \
  ../include/linux/rculist_bl.h \
  ../include/linux/list_bl.h \
  ../include/linux/bit_spinlock.h \
  ../include/linux/lockref.h \
    $(wildcard include/config/arch/use/cmpxchg/lockref.h) \
  ../include/linux/stringhash.h \
    $(wildcard include/config/dcache/word/access.h) \
  ../include/linux/hash.h \
    $(wildcard include/config/have/arch/hash.h) \
  ../include/linux/path.h \
  ../include/linux/list_lru.h \
  ../include/linux/shrinker.h \
  ../include/linux/mm_types.h \
    $(wildcard include/config/have/cmpxchg/double.h) \
    $(wildcard include/config/have/aligned/struct/page.h) \
    $(wildcard include/config/userfaultfd.h) \
    $(wildcard include/config/speculative/page/fault.h) \
    $(wildcard include/config/have/arch/compat/mmap/bases.h) \
    $(wildcard include/config/membarrier.h) \
    $(wildcard include/config/aio.h) \
    $(wildcard include/config/mmu/notifier.h) \
    $(wildcard include/config/hmm.h) \
  ../include/linux/auxvec.h \
  ../include/uapi/linux/auxvec.h \
  ../arch/arm64/include/uapi/asm/auxvec.h \
  ../include/linux/uprobes.h \
  ../arch/arm64/include/asm/uprobes.h \
  ../arch/arm64/include/asm/debug-monitors.h \
  ../arch/arm64/include/asm/esr.h \
  ../arch/arm64/include/asm/probes.h \
    $(wildcard include/config/kprobes.h) \
  ../arch/arm64/include/asm/mmu.h \
    $(wildcard include/config/unmap/kernel/at/el0.h) \
    $(wildcard include/config/harden/branch/predictor.h) \
  ../include/linux/capability.h \
  ../include/uapi/linux/capability.h \
  ../include/linux/semaphore.h \
  ../include/linux/fcntl.h \
  ../include/uapi/linux/fcntl.h \
    $(wildcard include/config/five/debug.h) \
  ../arch/arm64/include/uapi/asm/fcntl.h \
  ../include/uapi/asm-generic/fcntl.h \
  ../include/uapi/linux/fiemap.h \
  ../include/linux/migrate_mode.h \
  ../include/linux/percpu-rwsem.h \
  ../include/linux/rcuwait.h \
  ../include/linux/rcu_sync.h \
  ../include/linux/delayed_call.h \
  ../include/linux/uuid.h \
  ../include/uapi/linux/uuid.h \
  ../include/linux/errseq.h \
  ../include/uapi/linux/fs.h \
  ../include/uapi/linux/limits.h \
  ../include/uapi/linux/ioctl.h \
  arch/arm64/include/generated/uapi/asm/ioctl.h \
  ../include/asm-generic/ioctl.h \
  ../include/uapi/asm-generic/ioctl.h \
  ../include/linux/quota.h \
    $(wildcard include/config/quota/netlink/interface.h) \
  ../include/linux/percpu_counter.h \
  ../include/uapi/linux/dqblk_xfs.h \
  ../include/linux/dqblk_v1.h \
  ../include/linux/dqblk_v2.h \
  ../include/linux/dqblk_qtree.h \
  ../include/linux/projid.h \
  ../include/uapi/linux/quota.h \
  ../include/linux/nfs_fs_i.h \
  ../include/linux/cdev.h \
  ../include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
    $(wildcard include/config/generic/msi/irq/domain.h) \
    $(wildcard include/config/pinctrl.h) \
    $(wildcard include/config/generic/msi/irq.h) \
    $(wildcard include/config/dma/cma.h) \
    $(wildcard include/config/of.h) \
    $(wildcard include/config/devtmpfs.h) \
    $(wildcard include/config/sysfs/deprecated.h) \
  ../include/linux/ioport.h \
  ../include/linux/klist.h \
  ../include/linux/pinctrl/devinfo.h \
    $(wildcard include/config/pm.h) \
  ../include/linux/pinctrl/consumer.h \
  ../include/linux/seq_file.h \
  ../include/linux/cred.h \
    $(wildcard include/config/rustuh/kdp/cred.h) \
    $(wildcard include/config/debug/credentials.h) \
    $(wildcard include/config/keys.h) \
  ../include/linux/key.h \
  ../include/linux/assoc_array.h \
    $(wildcard include/config/associative/array.h) \
  ../include/linux/selinux.h \
    $(wildcard include/config/security/selinux.h) \
  ../include/linux/sched/user.h \
    $(wildcard include/config/fanotify.h) \
    $(wildcard include/config/posix/mqueue.h) \
    $(wildcard include/config/bpf/syscall.h) \
    $(wildcard include/config/net.h) \
  ../include/linux/pinctrl/pinctrl-state.h \
  ../include/linux/pm.h \
    $(wildcard include/config/vt/console/sleep.h) \
    $(wildcard include/config/pm/clk.h) \
    $(wildcard include/config/pm/generic/domains.h) \
  ../include/linux/ratelimit.h \
  ../arch/arm64/include/asm/device.h \
    $(wildcard include/config/iommu/api.h) \
    $(wildcard include/config/xen.h) \
  ../include/linux/pm_wakeup.h \
  ../include/linux/poll.h \
  ../include/linux/uaccess.h \
  ../arch/arm64/include/asm/uaccess.h \
    $(wildcard include/config/arm64/pan.h) \
  ../arch/arm64/include/asm/kernel-pgtable.h \
  ../arch/arm64/include/asm/pgtable.h \
    $(wildcard include/config/uh.h) \
  ../arch/arm64/include/asm/proc-fns.h \
  ../arch/arm64/include/asm/pgtable-prot.h \
  ../arch/arm64/include/asm/fixmap.h \
    $(wildcard include/config/acpi/apei/ghes.h) \
  ../arch/arm64/include/asm/boot.h \
  ../include/asm-generic/fixmap.h \
  ../include/asm-generic/pgtable.h \
    $(wildcard include/config/have/arch/transparent/hugepage/pud.h) \
    $(wildcard include/config/have/arch/soft/dirty.h) \
    $(wildcard include/config/arch/enable/thp/migration.h) \
    $(wildcard include/config/have/arch/huge/vmap.h) \
    $(wildcard include/config/x86/espfix64.h) \
  ../arch/arm64/include/asm/extable.h \
  ../include/uapi/linux/poll.h \
  arch/arm64/include/generated/uapi/asm/poll.h \
  ../include/uapi/asm-generic/poll.h \
  ../include/linux/proc_fs.h \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/proc/uid.h) \
  ../include/linux/delay.h \
  arch/arm64/include/generated/asm/delay.h \
  ../include/asm-generic/delay.h \
  ../include/linux/interrupt.h \
    $(wildcard include/config/irq/forced/threading.h) \
    $(wildcard include/config/generic/irq/probe.h) \
    $(wildcard include/config/irq/timings.h) \
  ../include/linux/irqreturn.h \
  ../include/linux/irqnr.h \
  ../include/uapi/linux/irqnr.h \
  ../include/linux/hardirq.h \
  ../include/linux/ftrace_irq.h \
    $(wildcard include/config/ftrace/nmi/enter.h) \
    $(wildcard include/config/hwlat/tracer.h) \
  ../include/linux/vtime.h \
    $(wildcard include/config/virt/cpu/accounting.h) \
    $(wildcard include/config/irq/time/accounting.h) \
  ../include/linux/context_tracking_state.h \
    $(wildcard include/config/context/tracking.h) \
  ../include/linux/static_key.h \
  ../arch/arm64/include/asm/hardirq.h \
  ../arch/arm64/include/asm/irq.h \
  ../include/asm-generic/irq.h \
  ../arch/arm64/include/asm/kvm_arm.h \
  ../include/linux/irq_cpustat.h \
  ../drivers/misc/mediatek/include/mt-plat/mtk_sched_mon.h \
    $(wildcard include/config/mtk/ram/console.h) \
    $(wildcard include/config/mtk/irq/count/tracer.h) \
    $(wildcard include/config/mtk/irq/off/tracer.h) \
    $(wildcard include/config/mtk/preempt/tracer.h) \
  ../include/linux/sched/clock.h \
    $(wildcard include/config/have/unstable/sched/clock.h) \
  ../drivers/misc/mediatek/include/mt-plat/mtk_ram_console.h \
    $(wildcard include/config/mtk/aee/ipanic.h) \
  ../include/linux/console.h \
    $(wildcard include/config/hw/console.h) \
    $(wildcard include/config/tty.h) \
    $(wildcard include/config/vga/console.h) \
  ../include/linux/pstore.h \
    $(wildcard include/config/arm/thumb.h) \
    $(wildcard include/config/arm.h) \
  ../include/linux/kmsg_dump.h \
  ../include/linux/vmalloc.h \
  ../include/linux/firmware.h \
    $(wildcard include/config/fw/loader.h) \
  ../include/linux/kthread.h \
  ../include/linux/slab.h \
    $(wildcard include/config/debug/slab.h) \
    $(wildcard include/config/failslab.h) \
    $(wildcard include/config/have/hardened/usercopy/allocator.h) \
    $(wildcard include/config/slab.h) \
    $(wildcard include/config/slub.h) \
  ../include/linux/kmemleak.h \
    $(wildcard include/config/debug/kmemleak.h) \
  ../include/linux/kasan.h \
    $(wildcard include/config/kasan/generic.h) \
  ../drivers/misc/mediatek/include/mt-plat/aee.h \
    $(wildcard include/config/mtk/printk/uart/console.h) \
    $(wildcard include/config/console/lock/duration/detect.h) \
    $(wildcard include/config/mtk/aee/dram/console.h) \
    $(wildcard include/config/mach/mt6763.h) \
  ../include/linux/platform_device.h \
    $(wildcard include/config/suspend.h) \
    $(wildcard include/config/hibernate/callbacks.h) \
  ../include/linux/mod_devicetable.h \
  ../include/linux/kfifo.h \
  ../include/linux/scatterlist.h \
    $(wildcard include/config/debug/sg.h) \
    $(wildcard include/config/need/sg/dma/length.h) \
    $(wildcard include/config/sgl/alloc.h) \
    $(wildcard include/config/arch/has/sg/chain.h) \
    $(wildcard include/config/sg/pool.h) \
  ../include/linux/mm.h \
    $(wildcard include/config/have/arch/mmap/rnd/bits.h) \
    $(wildcard include/config/have/arch/mmap/rnd/compat/bits.h) \
    $(wildcard include/config/mem/soft/dirty.h) \
    $(wildcard include/config/arch/uses/high/vma/flags.h) \
    $(wildcard include/config/x86.h) \
    $(wildcard include/config/x86/intel/memory/protection/keys.h) \
    $(wildcard include/config/ppc.h) \
    $(wildcard include/config/parisc.h) \
    $(wildcard include/config/metag.h) \
    $(wildcard include/config/x86/intel/mpx.h) \
    $(wildcard include/config/have/arch/userfaultfd/minor.h) \
    $(wildcard include/config/device/private.h) \
    $(wildcard include/config/device/public.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/debug/vm/rb.h) \
    $(wildcard include/config/page/poisoning.h) \
    $(wildcard include/config/init/on/alloc/default/on.h) \
    $(wildcard include/config/init/on/free/default/on.h) \
    $(wildcard include/config/debug/pagealloc.h) \
    $(wildcard include/config/hibernation.h) \
    $(wildcard include/config/hugetlbfs.h) \
  ../include/linux/range.h \
  ../include/linux/percpu-refcount.h \
  ../include/linux/page_ext.h \
    $(wildcard include/config/idle/page/tracking.h) \
  ../include/linux/stacktrace.h \
    $(wildcard include/config/stacktrace.h) \
    $(wildcard include/config/user/stacktrace/support.h) \
  ../include/linux/stackdepot.h \
  ../include/linux/page_ref.h \
    $(wildcard include/config/debug/page/ref.h) \
  ../include/linux/page-flags.h \
    $(wildcard include/config/arch/uses/pg/uncached.h) \
    $(wildcard include/config/memory/failure.h) \
    $(wildcard include/config/thp/swap.h) \
    $(wildcard include/config/ksm.h) \
  ../include/linux/tracepoint-defs.h \
  ../include/linux/memremap.h \
  ../include/linux/huge_mm.h \
  ../include/linux/sched/coredump.h \
    $(wildcard include/config/core/dump/default/elf/headers.h) \
  ../include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
    $(wildcard include/config/debug/tlbflush.h) \
    $(wildcard include/config/debug/vm/vmacache.h) \
  ../include/linux/vm_event_item.h \
    $(wildcard include/config/memory/balloon.h) \
    $(wildcard include/config/balloon/compaction.h) \
    $(wildcard include/config/zram/lru/writeback.h) \
  ../arch/arm64/include/asm/io.h \
  arch/arm64/include/generated/asm/early_ioremap.h \
  ../include/asm-generic/early_ioremap.h \
    $(wildcard include/config/generic/early/ioremap.h) \
  ../include/xen/xen.h \
    $(wildcard include/config/xen/pvh.h) \
    $(wildcard include/config/xen/dom0.h) \
  ../include/asm-generic/io.h \
    $(wildcard include/config/generic/iomap.h) \
    $(wildcard include/config/has/ioport/map.h) \
    $(wildcard include/config/virt/to/bus.h) \
  ../include/asm-generic/pci_iomap.h \
    $(wildcard include/config/pci.h) \
    $(wildcard include/config/no/generic/pci/ioport/map.h) \
    $(wildcard include/config/generic/pci/iomap.h) \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/wmt_dbg.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/osal.h \
    $(wildcard include/config/mtk/gmo/ram/optimize.h) \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/include/../../debug_utility/ring.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/wmt_dev.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include/wmt_lib.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include/wmt_core.h \
    $(wildcard include/config/mtk/combo/ant.h) \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include/wmt_ctrl.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/include/stp_exp.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/include/wmt_exp.h \
  ../drivers/misc/mediatek/include/mt-plat/mtk_wcn_cmb_stub.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/include/wmt_plat.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/include/mtk_wcn_cmb_hw.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include/stp_wmt.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/wmt_idc.h \
  ../drivers/misc/mediatek/connectivity/common/connectivity_build_in_adapter.h \
    $(wildcard include/config/mach/mt6735.h) \
    $(wildcard include/config/mach/mt6735m.h) \
    $(wildcard include/config/mach/mt6753.h) \
    $(wildcard include/config/mach/mt6739.h) \
    $(wildcard include/config/mach/mt6755.h) \
    $(wildcard include/config/mach/mt6757.h) \
    $(wildcard include/config/mach/mt6758.h) \
    $(wildcard include/config/mach/mt6759.h) \
    $(wildcard include/config/mach/mt6775.h) \
    $(wildcard include/config/mach/mt6797.h) \
    $(wildcard include/config/mach/mt6799.h) \
    $(wildcard include/config/mach/mt6580.h) \
    $(wildcard include/config/mach/mt6765.h) \
    $(wildcard include/config/mach/mt6761.h) \
    $(wildcard include/config/mach/mt3967.h) \
    $(wildcard include/config/mach/mt6771.h) \
    $(wildcard include/config/mach/mt6768.h) \
    $(wildcard include/config/mach/mt6785.h) \
    $(wildcard include/config/mach/kiboplus.h) \
    $(wildcard include/config/mach/mt6885.h) \
    $(wildcard include/config/mach/mt6853.h) \
    $(wildcard include/config/mach/elbrus.h) \
    $(wildcard include/config/mach/mt6893.h) \
    $(wildcard include/config/mtk/pmic/chip/mt6359.h) \
    $(wildcard include/config/mtk/pmic/chip/mt6359p.h) \
    $(wildcard include/config/mtk/mt6306/gpio/support.h) \
    $(wildcard include/config/arch/mt6570.h) \
    $(wildcard include/config/arch/mt6755.h) \
    $(wildcard include/config/mtk/gpio.h) \
    $(wildcard include/config/mach/mt8167.h) \
  ../include/linux/dma-mapping.h \
    $(wildcard include/config/have/generic/dma/coherent.h) \
    $(wildcard include/config/has/dma.h) \
    $(wildcard include/config/arch/has/dma/set/coherent/mask.h) \
    $(wildcard include/config/need/dma/map/state.h) \
    $(wildcard include/config/dma/api/debug.h) \
  ../include/linux/dma-debug.h \
  ../include/linux/dma-direction.h \
  ../include/linux/mem_encrypt.h \
    $(wildcard include/config/arch/has/mem/encrypt.h) \
    $(wildcard include/config/amd/mem/encrypt.h) \
  ../arch/arm64/include/asm/dma-mapping.h \
    $(wildcard include/config/mtk/bouncing/check.h) \
    $(wildcard include/config/iommu/dma.h) \
  ../arch/arm64/include/asm/xen/hypervisor.h \
  ../include/xen/arm/hypervisor.h \
  ../arch/arm64/include/asm/../../../../drivers/misc/mediatek/include/mt-plat/aee.h \
  ../drivers/misc/mediatek/conn_md/include/conn_md_exp.h \
  ../drivers/misc/mediatek/eccci/port_ipc.h \
    $(wildcard include/config/mtk/md/direct/tethering/support.h) \
    $(wildcard include/config/mtk/md/direct/logging/support.h) \
  ../drivers/misc/mediatek/eccci/mt6768/ccci_config.h \
  ../drivers/misc/mediatek/eccci/ccci_ipc_task_ID.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/platform/include/mtk_wcn_consys_hw.h \
    $(wildcard include/config/new/opid.h) \
    $(wildcard include/config/adjust.h) \
    $(wildcard include/config/adjust/new/flag.h) \
    $(wildcard include/config/mtk/gpio/legacy.h) \
  ../include/linux/of.h \
    $(wildcard include/config/sparc.h) \
    $(wildcard include/config/of/dynamic.h) \
    $(wildcard include/config/attach/node.h) \
    $(wildcard include/config/detach/node.h) \
    $(wildcard include/config/add/property.h) \
    $(wildcard include/config/remove/property.h) \
    $(wildcard include/config/update/property.h) \
    $(wildcard include/config/of/numa.h) \
    $(wildcard include/config/no/change.h) \
    $(wildcard include/config/change/add.h) \
    $(wildcard include/config/change/remove.h) \
    $(wildcard include/config/of/resolve.h) \
    $(wildcard include/config/of/overlay.h) \
  ../include/linux/property.h \
  ../include/linux/fwnode.h \
  ../include/linux/of_irq.h \
    $(wildcard include/config/ppc32.h) \
    $(wildcard include/config/ppc/pmac.h) \
    $(wildcard include/config/of/irq.h) \
  ../include/linux/irq.h \
    $(wildcard include/config/generic/irq/effective/aff/mask.h) \
    $(wildcard include/config/generic/irq/ipi.h) \
    $(wildcard include/config/irq/domain/hierarchy.h) \
    $(wildcard include/config/generic/irq/migration.h) \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/hardirqs/sw/resend.h) \
    $(wildcard include/config/generic/irq/legacy/alloc/hwirq.h) \
    $(wildcard include/config/generic/irq/legacy.h) \
  ../include/linux/irqhandler.h \
  ../include/linux/io.h \
  arch/arm64/include/generated/asm/irq_regs.h \
  ../include/asm-generic/irq_regs.h \
  ../include/linux/irqdesc.h \
    $(wildcard include/config/irq/preflow/fasteoi.h) \
    $(wildcard include/config/generic/irq/debugfs.h) \
    $(wildcard include/config/sparse/irq.h) \
    $(wildcard include/config/handle/domain/irq.h) \
  arch/arm64/include/generated/asm/hw_irq.h \
  ../include/asm-generic/hw_irq.h \
  ../include/linux/irqdomain.h \
    $(wildcard include/config/irq/domain.h) \
  ../include/linux/of_address.h \
    $(wildcard include/config/of/address.h) \
  ../include/linux/pm_runtime.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include/wmt_conf.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/include/wmt_plat_stub.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_detect/wmt_detect.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_detect/sdio_detect.h \
  ../include/linux/mmc/card.h \
    $(wildcard include/config/mmc/ffu.h) \
    $(wildcard include/config/mtk/emmc/hw/cq.h) \
    $(wildcard include/config/mtk/emmc/cq/support.h) \
  ../include/linux/mmc/host.h \
    $(wildcard include/config/regulator.h) \
    $(wildcard include/config/fail/mmc/request.h) \
    $(wildcard include/config/mmc/crypto.h) \
    $(wildcard include/config/mmc/embedded/sdio.h) \
  ../include/linux/fault-inject.h \
    $(wildcard include/config/fault/injection/debug/fs.h) \
  ../include/linux/debugfs.h \
    $(wildcard include/config/debug/fs.h) \
  ../include/linux/mmc/core.h \
    $(wildcard include/config/mtk/hw/fde.h) \
  ../include/linux/mmc/pm.h \
  ../include/linux/mmc/sdio_func.h \
  ../include/linux/mmc/sdio_ids.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_detect/wmt_detect_pwr.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include/stp_core.h \
    $(wildcard include/config/power/saving/support.h) \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include/psm_core.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/include/btm_core.h \
  ../drivers/misc/mediatek/btif/common/inc/mtk_btif_exp.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/stp_btif.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/stp_sdio.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/hif_sdio.h \
  ../include/linux/mmc/sdio.h \
  ../drivers/mmc/core/sdio_ops.h \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/stp_dbg.h \
    $(wildcard include/config/log/stp/internal.h) \
  ../drivers/misc/mediatek/connectivity/wmt_drv/common_main/linux/include/wmt_step.h \
    $(wildcard include/config/name.h) \
    $(wildcard include/config/base.h) \
  ../include/linux/rtc.h \
    $(wildcard include/config/rtc/intf/dev/uie/emul.h) \
    $(wildcard include/config/rtc/hctosys/device.h) \
  ../include/linux/nvmem-provider.h \
    $(wildcard include/config/nvmem.h) \
  ../include/uapi/linux/rtc.h \

drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/wmt_lib.o: $(deps_drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/wmt_lib.o)

$(deps_drivers/misc/mediatek/connectivity/wmt_drv/common_main/core/wmt_lib.o):
