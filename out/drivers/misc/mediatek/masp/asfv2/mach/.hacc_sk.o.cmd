cmd_drivers/misc/mediatek/masp/asfv2/mach/hacc_sk.o := /home/luminous418/zyc-clang/bin/clang -Wp,-MD,drivers/misc/mediatek/masp/asfv2/mach/.hacc_sk.o.d -nostdinc -isystem /home/luminous418/zyc-clang/lib/clang/14.0.6/include -I../arch/arm64/include -I./arch/arm64/include/generated  -I../include -I../drivers/misc/mediatek/include -I./include -I../arch/arm64/include/uapi -I./arch/arm64/include/generated/uapi -I../include/uapi -I./include/generated/uapi -include ../include/linux/kconfig.h  -I../drivers/misc/mediatek/masp -Idrivers/misc/mediatek/masp -D__KERNEL__ -Qunused-arguments -mlittle-endian -DKASAN_SHADOW_SCALE_SHIFT=3 -O3 -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 --target=aarch64-linux-gnu --prefix=/home/luminous418/zyc-clang/bin/aarch64-linux-gnu- --gcc-toolchain=/home/luminous418/zyc-clang -no-integrated-as -Werror=unknown-warning-option -fuse-ld=lld -fno-PIE -mno-implicit-float -DCONFIG_AS_LSE=1 -fno-asynchronous-unwind-tables -fno-pic -Wno-asm-operand-widths -DKASAN_SHADOW_SCALE_SHIFT=3 -fno-delete-null-pointer-checks -Wno-frame-address -Wno-int-in-bool-context -Wno-address-of-packed-member -O3 --param=allow-store-data-races=0 -mcpu=cortex-a55 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=2800 -fstack-protector-strong -Wno-format-invalid-specifier -Wno-gnu -Wno-duplicate-decl-specifier -Wno-tautological-compare -mno-global-merge -Wno-unused-but-set-variable -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -Wno-array-bounds -fno-strict-overflow -fno-merge-all-constants -fno-stack-check -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -fmacro-prefix-map=../= -Wno-initializer-overrides -Wno-unused-value -Wno-format -Wno-sign-compare -Wno-format-zero-length -Wno-uninitialized -Wno-pointer-to-enum-cast -Wno-unaligned-access -Wno-enum-compare-conditional -Wno-enum-enum-conversion -w -pipe -O3 -Werror  -I../drivers/misc/mediatek/include  -I../drivers/misc/mediatek/include/mt-plat/mt6768/include/  -I../drivers/misc/mediatek/include/mt-plat/  -I../drivers/mmc/host/mediatek/mt6768  -I../drivers/misc/mediatek/masp/asfv2/asf_inc  -I../drivers/misc/mediatek/masp/asfv2/asf_export_inc  -I../drivers/misc/mediatek/masp/asfv2/module  -I../drivers/misc/mediatek/masp/asfv2/asf_inc    -DKBUILD_BASENAME='"hacc_sk"'  -DKBUILD_MODNAME='"sec"' -c -o drivers/misc/mediatek/masp/asfv2/mach/.tmp_hacc_sk.o ../drivers/misc/mediatek/masp/asfv2/mach/hacc_sk.c

source_drivers/misc/mediatek/masp/asfv2/mach/hacc_sk.o := ../drivers/misc/mediatek/masp/asfv2/mach/hacc_sk.c

deps_drivers/misc/mediatek/masp/asfv2/mach/hacc_sk.o := \
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
  ../drivers/misc/mediatek/masp/asfv2/asf_inc/sec_error.h \
  ../drivers/misc/mediatek/masp/asfv2/mach/hacc_mach.h \
  ../drivers/misc/mediatek/masp/asfv2/asf_inc/sec_osal_light.h \
  ../include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
    $(wildcard include/config/fortify/source.h) \
    $(wildcard include/config/kasan.h) \
  ../include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/stack/validation.h) \
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
  /home/luminous418/zyc-clang/lib/clang/14.0.6/include/stdarg.h \
  ../include/uapi/linux/string.h \
  ../arch/arm64/include/asm/string.h \
    $(wildcard include/config/arch/has/uaccess/flushcache.h) \
  ../include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/arch/has/refcount.h) \
    $(wildcard include/config/panic/timeout.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/disable/trace/printk.h) \
  ../include/linux/linkage.h \
    $(wildcard include/config/rustuh/rkp.h) \
    $(wildcard include/config/uh/rkp.h) \
    $(wildcard include/config/rustuh/kdp.h) \
    $(wildcard include/config/kdp/cred.h) \
  ../include/linux/stringify.h \
  ../include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/module/rel/crcs.h) \
    $(wildcard include/config/trim/unused/ksyms.h) \
    $(wildcard include/config/sec/kunit.h) \
    $(wildcard include/config/kunit.h) \
    $(wildcard include/config/unused/symbols.h) \
  ../arch/arm64/include/asm/linkage.h \
  ../include/linux/bitops.h \
  ../include/linux/bits.h \
  ../include/linux/const.h \
  ../include/uapi/linux/const.h \
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
    $(wildcard include/config/mtk/eng/build.h) \
    $(wildcard include/config/printk/mt/prefix.h) \
    $(wildcard include/config/sec/debug/auto/comment.h) \
    $(wildcard include/config/message/loglevel/default.h) \
    $(wildcard include/config/console/loglevel/default.h) \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk/nmi.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  ../include/linux/init.h \
    $(wildcard include/config/strict/kernel/rwx.h) \
    $(wildcard include/config/strict/module/rwx.h) \
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
  /home/luminous418/zyc-clang/lib/clang/14.0.6/include/stdbool.h \
  ../include/linux/io.h \
    $(wildcard include/config/have/arch/huge/vmap.h) \
    $(wildcard include/config/has/ioport/map.h) \
    $(wildcard include/config/pci.h) \
  ../include/linux/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/bug/on/data/corruption.h) \
  ../arch/arm64/include/asm/bug.h \
  ../arch/arm64/include/asm/asm-bug.h \
    $(wildcard include/config/debug/bugverbose.h) \
  ../arch/arm64/include/asm/brk-imm.h \
  ../include/asm-generic/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  ../include/linux/err.h \
  arch/arm64/include/generated/uapi/asm/errno.h \
  ../include/uapi/asm-generic/errno.h \
  ../include/uapi/asm-generic/errno-base.h \
  ../arch/arm64/include/asm/io.h \
  ../arch/arm64/include/asm/memory.h \
    $(wildcard include/config/arm64/va/bits.h) \
    $(wildcard include/config/vmap/stack.h) \
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
  ../arch/arm64/include/asm/pgtable.h \
    $(wildcard include/config/uh.h) \
    $(wildcard include/config/numa/balancing.h) \
    $(wildcard include/config/transparent/hugepage.h) \
    $(wildcard include/config/pgtable/levels.h) \
  ../arch/arm64/include/asm/proc-fns.h \
  ../arch/arm64/include/asm/page.h \
    $(wildcard include/config/have/arch/pfn/valid.h) \
  ../include/linux/personality.h \
  ../include/uapi/linux/personality.h \
  ../arch/arm64/include/asm/pgtable-types.h \
  ../include/asm-generic/pgtable-nopud.h \
  ../include/asm-generic/pgtable-nop4d-hack.h \
  ../include/asm-generic/5level-fixup.h \
  ../include/asm-generic/getorder.h \
  ../arch/arm64/include/asm/pgtable-hwdef.h \
  ../arch/arm64/include/asm/pgtable-prot.h \
  ../arch/arm64/include/asm/cmpxchg.h \
  ../arch/arm64/include/asm/atomic.h \
    $(wildcard include/config/arm64/lse/atomics.h) \
    $(wildcard include/config/as/lse.h) \
  ../arch/arm64/include/asm/lse.h \
  ../arch/arm64/include/asm/atomic_ll_sc.h \
  ../arch/arm64/include/asm/fixmap.h \
    $(wildcard include/config/acpi/apei/ghes.h) \
    $(wildcard include/config/unmap/kernel/at/el0.h) \
  ../arch/arm64/include/asm/boot.h \
  ../include/asm-generic/fixmap.h \
  ../include/asm-generic/pgtable.h \
    $(wildcard include/config/have/arch/transparent/hugepage/pud.h) \
    $(wildcard include/config/have/arch/soft/dirty.h) \
    $(wildcard include/config/arch/enable/thp/migration.h) \
    $(wildcard include/config/x86/espfix64.h) \
  ../include/linux/mm_types.h \
    $(wildcard include/config/have/cmpxchg/double.h) \
    $(wildcard include/config/have/aligned/struct/page.h) \
    $(wildcard include/config/memcg.h) \
    $(wildcard include/config/userfaultfd.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/speculative/page/fault.h) \
    $(wildcard include/config/have/arch/compat/mmap/bases.h) \
    $(wildcard include/config/membarrier.h) \
    $(wildcard include/config/aio.h) \
    $(wildcard include/config/mmu/notifier.h) \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/arch/want/batched/unmap/tlb/flush.h) \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hmm.h) \
  ../include/linux/mm_types_task.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/arch/enable/split/pmd/ptlock.h) \
  ../include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  ../include/linux/atomic.h \
    $(wildcard include/config/generic/atomic64.h) \
  ../include/asm-generic/atomic-long.h \
  ../include/linux/cpumask.h \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  ../include/linux/bitmap.h \
  ../include/linux/auxvec.h \
  ../include/uapi/linux/auxvec.h \
  ../arch/arm64/include/uapi/asm/auxvec.h \
  ../include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  ../include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
    $(wildcard include/config/page/poisoning/zero.h) \
  ../include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  ../include/linux/preempt.h \
    $(wildcard include/config/preempt/count.h) \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  arch/arm64/include/generated/asm/preempt.h \
  ../include/asm-generic/preempt.h \
  ../include/linux/thread_info.h \
    $(wildcard include/config/thread/info/in/task.h) \
    $(wildcard include/config/have/arch/within/stack/frames.h) \
    $(wildcard include/config/hardened/usercopy.h) \
  ../include/linux/restart_block.h \
    $(wildcard include/config/compat.h) \
  ../include/linux/errno.h \
  ../include/uapi/linux/errno.h \
  ../arch/arm64/include/asm/current.h \
  ../arch/arm64/include/asm/thread_info.h \
    $(wildcard include/config/arm64/sw/ttbr0/pan.h) \
    $(wildcard include/config/shadow/call/stack.h) \
  ../arch/arm64/include/asm/stack_pointer.h \
  ../include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  ../arch/arm64/include/asm/irqflags.h \
  ../arch/arm64/include/asm/ptrace.h \
  ../arch/arm64/include/uapi/asm/ptrace.h \
  ../arch/arm64/include/asm/hwcap.h \
  ../arch/arm64/include/uapi/asm/hwcap.h \
  ../include/asm-generic/ptrace.h \
  ../include/linux/bottom_half.h \
  ../include/linux/spinlock_types.h \
  ../arch/arm64/include/asm/spinlock_types.h \
  ../include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/lockdep/crossrelease.h) \
  ../include/linux/rwlock_types.h \
  ../arch/arm64/include/asm/spinlock.h \
  ../arch/arm64/include/asm/processor.h \
    $(wildcard include/config/have/hw/breakpoint.h) \
    $(wildcard include/config/arm64/tagged/addr/abi.h) \
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
  ../include/linux/rbtree.h \
  ../include/linux/rcupdate.h \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/rcu/stall/common.h) \
    $(wildcard include/config/no/hz/full.h) \
    $(wildcard include/config/rcu/nocb/cpu.h) \
    $(wildcard include/config/tasks/rcu.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/tiny/rcu.h) \
    $(wildcard include/config/debug/objects/rcu/head.h) \
    $(wildcard include/config/prove/rcu.h) \
    $(wildcard include/config/rcu/boost.h) \
    $(wildcard include/config/arch/weak/release/acquire.h) \
  ../include/linux/rcutree.h \
  ../include/linux/rwsem.h \
    $(wildcard include/config/rwsem/spin/on/owner.h) \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
    $(wildcard include/config/mtk/task/turbo.h) \
  ../include/linux/osq_lock.h \
  arch/arm64/include/generated/asm/rwsem.h \
  ../include/asm-generic/rwsem.h \
  ../include/linux/completion.h \
    $(wildcard include/config/lockdep/completions.h) \
  ../include/linux/wait.h \
  ../include/uapi/linux/wait.h \
  ../include/linux/uprobes.h \
    $(wildcard include/config/uprobes.h) \
  ../arch/arm64/include/asm/uprobes.h \
  ../arch/arm64/include/asm/debug-monitors.h \
  ../arch/arm64/include/asm/esr.h \
  ../arch/arm64/include/asm/probes.h \
    $(wildcard include/config/kprobes.h) \
  ../include/linux/page-flags-layout.h \
  ../include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/generated/bounds.h \
  ../arch/arm64/include/asm/sparsemem.h \
  ../include/linux/workqueue.h \
    $(wildcard include/config/debug/objects/work.h) \
    $(wildcard include/config/freezer.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/wq/watchdog.h) \
  ../include/linux/timer.h \
    $(wildcard include/config/cfi/clang.h) \
    $(wildcard include/config/debug/objects/timers.h) \
    $(wildcard include/config/no/hz/common.h) \
  ../include/linux/ktime.h \
  ../include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  ../include/linux/seqlock.h \
  ../include/linux/math64.h \
    $(wildcard include/config/arch/supports/int128.h) \
  arch/arm64/include/generated/asm/div64.h \
  ../include/asm-generic/div64.h \
  ../include/linux/time64.h \
  ../include/uapi/linux/time.h \
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
    $(wildcard include/config/mtk/rt/throttle/mon.h) \
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
  ../arch/arm64/include/asm/mmu.h \
    $(wildcard include/config/harden/branch/predictor.h) \
  arch/arm64/include/generated/asm/early_ioremap.h \
  ../include/asm-generic/early_ioremap.h \
    $(wildcard include/config/generic/early/ioremap.h) \
  ../include/xen/xen.h \
    $(wildcard include/config/xen.h) \
    $(wildcard include/config/xen/pvh.h) \
    $(wildcard include/config/xen/dom0.h) \
  ../include/asm-generic/io.h \
    $(wildcard include/config/generic/iomap.h) \
    $(wildcard include/config/virt/to/bus.h) \
  ../include/asm-generic/pci_iomap.h \
    $(wildcard include/config/no/generic/pci/ioport/map.h) \
    $(wildcard include/config/generic/pci/iomap.h) \
  ../include/linux/vmalloc.h \

drivers/misc/mediatek/masp/asfv2/mach/hacc_sk.o: $(deps_drivers/misc/mediatek/masp/asfv2/mach/hacc_sk.o)

$(deps_drivers/misc/mediatek/masp/asfv2/mach/hacc_sk.o):
