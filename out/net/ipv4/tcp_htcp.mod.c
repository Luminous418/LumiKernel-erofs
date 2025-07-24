#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x5821d3c0, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xd61d1578, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0xf1969a8e, __VMLINUX_SYMBOL_STR(__usecs_to_jiffies) },
	{ 0xcb59d26b, __VMLINUX_SYMBOL_STR(tcp_reno_undo_cwnd) },
	{ 0xc188a7c5, __VMLINUX_SYMBOL_STR(tcp_slow_start) },
	{ 0x526c3a6c, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xf92fec7a, __VMLINUX_SYMBOL_STR(tcp_unregister_congestion_control) },
	{ 0x21c3721b, __VMLINUX_SYMBOL_STR(tcp_register_congestion_control) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "90ED04813A2E07C3FEED99F");
