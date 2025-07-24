cmd_drivers/misc/mediatek/ccci_util/ccci_util_ld_md_errno.o := /home/luminous418/zyc-clang/bin/clang -Wp,-MD,drivers/misc/mediatek/ccci_util/.ccci_util_ld_md_errno.o.d -nostdinc -isystem /home/luminous418/zyc-clang/lib/clang/14.0.6/include -I../arch/arm64/include -I./arch/arm64/include/generated  -I../include -I../drivers/misc/mediatek/include -I./include -I../arch/arm64/include/uapi -I./arch/arm64/include/generated/uapi -I../include/uapi -I./include/generated/uapi -include ../include/linux/kconfig.h  -I../drivers/misc/mediatek/ccci_util -Idrivers/misc/mediatek/ccci_util -D__KERNEL__ -Qunused-arguments -mlittle-endian -DKASAN_SHADOW_SCALE_SHIFT=3 -O3 -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 --target=aarch64-linux-gnu --prefix=/home/luminous418/zyc-clang/bin/aarch64-linux-gnu- --gcc-toolchain=/home/luminous418/zyc-clang -no-integrated-as -Werror=unknown-warning-option -fuse-ld=lld -fno-PIE -mno-implicit-float -DCONFIG_AS_LSE=1 -fno-asynchronous-unwind-tables -fno-pic -Wno-asm-operand-widths -DKASAN_SHADOW_SCALE_SHIFT=3 -fno-delete-null-pointer-checks -Wno-frame-address -Wno-int-in-bool-context -Wno-address-of-packed-member -O3 --param=allow-store-data-races=0 -mcpu=cortex-a55 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=2800 -fstack-protector-strong -Wno-format-invalid-specifier -Wno-gnu -Wno-duplicate-decl-specifier -Wno-tautological-compare -mno-global-merge -Wno-unused-but-set-variable -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -Wno-array-bounds -fno-strict-overflow -fno-merge-all-constants -fno-stack-check -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -fmacro-prefix-map=../= -Wno-initializer-overrides -Wno-unused-value -Wno-format -Wno-sign-compare -Wno-format-zero-length -Wno-uninitialized -Wno-pointer-to-enum-cast -Wno-unaligned-access -Wno-enum-compare-conditional -Wno-enum-enum-conversion -w -pipe -O3 -Werror  -I../drivers/misc/mediatek/include  -I../drivers/misc/mediatek/include/mt-plat/mt6768/include/  -I../drivers/misc/mediatek/include/mt-plat/  -I../drivers/mmc/host/mediatek/mt6768  -I../drivers/misc/mediatek/ccci_util  -I../drivers/misc/mediatek/aee/mrdump/  -I../drivers/misc/mediatek/masp/asfv2/asf_export_inc -DENABLE_MD_IMG_SECURITY_FEATURE    -DKBUILD_BASENAME='"ccci_util_ld_md_errno"'  -DKBUILD_MODNAME='"ccci_util_lib"' -c -o drivers/misc/mediatek/ccci_util/.tmp_ccci_util_ld_md_errno.o ../drivers/misc/mediatek/ccci_util/ccci_util_ld_md_errno.c

source_drivers/misc/mediatek/ccci_util/ccci_util_ld_md_errno.o := ../drivers/misc/mediatek/ccci_util/ccci_util_ld_md_errno.c

deps_drivers/misc/mediatek/ccci_util/ccci_util_ld_md_errno.o := \
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

drivers/misc/mediatek/ccci_util/ccci_util_ld_md_errno.o: $(deps_drivers/misc/mediatek/ccci_util/ccci_util_ld_md_errno.o)

$(deps_drivers/misc/mediatek/ccci_util/ccci_util_ld_md_errno.o):
