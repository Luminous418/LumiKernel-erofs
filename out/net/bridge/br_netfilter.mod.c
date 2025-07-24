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
	{ 0x8a997e1d, __VMLINUX_SYMBOL_STR(proc_dointvec) },
	{ 0x6b2dc060, __VMLINUX_SYMBOL_STR(dump_stack) },
	{ 0x50388fde, __VMLINUX_SYMBOL_STR(nf_register_net_hooks) },
	{ 0x85670f1d, __VMLINUX_SYMBOL_STR(rtnl_is_locked) },
	{ 0xf52a376, __VMLINUX_SYMBOL_STR(pskb_expand_head) },
	{ 0x43b0c9c3, __VMLINUX_SYMBOL_STR(preempt_schedule) },
	{ 0x84280233, __VMLINUX_SYMBOL_STR(ip_do_fragment) },
	{ 0xcca54e8, __VMLINUX_SYMBOL_STR(nf_ipv6_ops) },
	{ 0xdccb0ce0, __VMLINUX_SYMBOL_STR(br_dev_queue_push_xmit) },
	{ 0x4829a47e, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x54ef9cc9, __VMLINUX_SYMBOL_STR(br_forward_finish) },
	{ 0xc8310237, __VMLINUX_SYMBOL_STR(skb_pull) },
	{ 0xf84066ae, __VMLINUX_SYMBOL_STR(nf_unregister_net_hooks) },
	{ 0x44406159, __VMLINUX_SYMBOL_STR(skb_push) },
	{ 0x376dda2f, __VMLINUX_SYMBOL_STR(ip_route_output_flow) },
	{ 0xffe74521, __VMLINUX_SYMBOL_STR(ip_route_input_noref) },
	{ 0xb7a0d63f, __VMLINUX_SYMBOL_STR(dst_release) },
	{ 0x9309f3fc, __VMLINUX_SYMBOL_STR(pskb_trim_rcsum_slow) },
	{ 0x26faba50, __VMLINUX_SYMBOL_STR(unregister_net_sysctl_table) },
	{ 0x9d0d6206, __VMLINUX_SYMBOL_STR(unregister_netdevice_notifier) },
	{ 0xfdd0720e, __VMLINUX_SYMBOL_STR(unregister_pernet_subsys) },
	{ 0x985558a1, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x19452d5b, __VMLINUX_SYMBOL_STR(nf_br_ops) },
	{ 0xfbebbd57, __VMLINUX_SYMBOL_STR(register_net_sysctl) },
	{ 0xf355e2a1, __VMLINUX_SYMBOL_STR(init_net) },
	{ 0xd2da1048, __VMLINUX_SYMBOL_STR(register_netdevice_notifier) },
	{ 0x54502430, __VMLINUX_SYMBOL_STR(register_pernet_subsys) },
	{ 0xc9bc5a0, __VMLINUX_SYMBOL_STR(__pskb_pull_tail) },
	{ 0x2469810f, __VMLINUX_SYMBOL_STR(__rcu_read_unlock) },
	{ 0x8d522714, __VMLINUX_SYMBOL_STR(__rcu_read_lock) },
	{ 0x9fec5ef3, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xf8e49c1e, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xdfeba5dd, __VMLINUX_SYMBOL_STR(skb_pull_rcsum) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xae6ce09a, __VMLINUX_SYMBOL_STR(nf_hook_slow) },
	{ 0x8f678b07, __VMLINUX_SYMBOL_STR(__stack_chk_guard) },
	{ 0x1fefead9, __VMLINUX_SYMBOL_STR(__vlan_find_dev_deep_rcu) },
	{ 0xaf06034a, __VMLINUX_SYMBOL_STR(neigh_destroy) },
	{ 0x733fd964, __VMLINUX_SYMBOL_STR(br_handle_frame_finish) },
	{ 0xadf12b77, __VMLINUX_SYMBOL_STR(kfree_skb) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "3DC49F2945EFB90B9430D83");
