cmd_firmware/melfas/melfas_mip4_ts.fw.gen.o := /home/luminous418/zyc-clang/bin/clang -Wp,-MD,firmware/melfas/.melfas_mip4_ts.fw.gen.o.d -nostdinc -isystem /home/luminous418/zyc-clang/lib/clang/14.0.6/include -I../arch/arm64/include -I./arch/arm64/include/generated  -I../include -I../drivers/misc/mediatek/include -I./include -I../arch/arm64/include/uapi -I./arch/arm64/include/generated/uapi -I../include/uapi -I./include/generated/uapi -include ../include/linux/kconfig.h -D__KERNEL__ -Qunused-arguments -mlittle-endian -DKASAN_SHADOW_SCALE_SHIFT=3 -O3 -D__ASSEMBLY__ --target=aarch64-linux-gnu --prefix=/home/luminous418/zyc-clang/bin/aarch64-linux-gnu- --gcc-toolchain=/home/luminous418/zyc-clang -no-integrated-as -Werror=unknown-warning-option -fno-PIE -DCONFIG_AS_LSE=1 -DKASAN_SHADOW_SCALE_SHIFT=3 -mcpu=cortex-a55 -DCC_HAVE_ASM_GOTO   -c -o firmware/melfas/melfas_mip4_ts.fw.gen.o firmware/melfas/melfas_mip4_ts.fw.gen.S

source_firmware/melfas/melfas_mip4_ts.fw.gen.o := firmware/melfas/melfas_mip4_ts.fw.gen.S

deps_firmware/melfas/melfas_mip4_ts.fw.gen.o := \
  ../include/linux/compiler_types.h \
    $(wildcard include/config/have/arch/compiler/h.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \

firmware/melfas/melfas_mip4_ts.fw.gen.o: $(deps_firmware/melfas/melfas_mip4_ts.fw.gen.o)

$(deps_firmware/melfas/melfas_mip4_ts.fw.gen.o):
