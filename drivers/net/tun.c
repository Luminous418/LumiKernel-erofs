/*
 *  TUN - Universal TUN/TAP device driver.
 *  Copyright (C) 1999-2002 Maxim Krasnyansky <maxk@qualcomm.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  $Id: tun.c,v 1.15 2002/03/01 02:44:24 maxk Exp $
 */

/*
 *  Changes:
 *
 *  Mike Kershaw <dragorn@kismetwireless.net> 2005/08/14
 *    Add TUNSETLINK ioctl to set the link encapsulation
 *
 *  Mark Smith <markzzzsmith@yahoo.com.au>
 *    Use eth_random_addr() for tap MAC address.
 *
 *  Harald Roelle <harald.roelle@ifi.lmu.de>  2004/04/20
 *    Fixes in packet dropping, queue length setting and queue wakeup.
 *    Increased default tx queue length.
 *    Added ethtool API.
 *    Minor cleanups
 *
 *  Daniel Podlejski <underley@underley.eu.org>
 *    Modifications for 2.3.99-pre5 kernel.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define DRV_NAME	"tun"
#define DRV_VERSION	"1.6"
#define DRV_DESCRIPTION	"Universal TUN/TAP device driver"
#define DRV_COPYRIGHT	"(C) 1999-2004 Max Krasnyansky <maxk@qualcomm.com>"

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/miscdevice.h>
#include <linux/ethtool.h>
#include <linux/rtnetlink.h>
#include <linux/compat.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/nsproxy.h>
#include <linux/virtio_net.h>
#include <linux/rcupdate.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <net/rtnetlink.h>
#include <net/sock.h>
#include <linux/seq_file.h>
#include <linux/uio.h>
#include <linux/skb_array.h>
// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
#include <linux/types.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <net/ip.h>

#define META_MARK_BASE_LOWER 100
#define META_MARK_BASE_UPPER 500

// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }

#include <linux/bpf.h>
#include <linux/bpf_trace.h>
#include <linux/ieee802154.h>
#include <linux/if_ltalk.h>
#include <uapi/linux/if_fddi.h>
#include <uapi/linux/if_hippi.h>
#include <uapi/linux/if_fc.h>
#include <net/ax25.h>
#include <net/rose.h>
#include <net/6lowpan.h>

#include <linux/uaccess.h>

/* Uncomment to enable debugging */
/* #define TUN_DEBUG 1 */

#ifdef TUN_DEBUG
static int debug;

#define tun_debug(level, tun, fmt, args...)			\
do {								\
	if (tun->debug)						\
		netdev_printk(level, tun->dev, fmt, ##args);	\
} while (0)
#define DBG1(level, fmt, args...)				\
do {								\
	if (debug == 2)						\
		printk(level fmt, ##args);			\
} while (0)
#else
#define tun_debug(level, tun, fmt, args...)			\
do {								\
	if (0)							\
		netdev_printk(level, tun->dev, fmt, ##args);	\
} while (0)
#define DBG1(level, fmt, args...)				\
do {								\
	if (0)							\
		printk(level fmt, ##args);			\
} while (0)
#endif

#define TUN_HEADROOM 256
#define TUN_RX_PAD (NET_IP_ALIGN + NET_SKB_PAD)

/* TUN device flags */

/* IFF_ATTACH_QUEUE is never stored in device flags,
 * overload it to mean fasync when stored there.
 */
#define TUN_FASYNC	IFF_ATTACH_QUEUE
/* High bits in flags field are unused. */
#define TUN_VNET_LE     0x80000000
#define TUN_VNET_BE     0x40000000

#define TUN_FEATURES (IFF_NO_PI | IFF_ONE_QUEUE | IFF_VNET_HDR | \
		      IFF_MULTI_QUEUE)

#define GOODCOPY_LEN 128

// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
/* The KNOX framework marks packets intended to a VPN client for special processing differently.
 * The marked packets hit special IP table rules and are routed back to user space using the TUN driver
 * for policy based treatment by the VPN client.
 * Some VPN clients can make more intelligent decisions based on the UID/PID information.
 * For such clients, we mark packets to be in the range >= META_MARK_BASE_LOWER and < META_MARK_BASE_UPPER.
 * When such packets are seen, we update the IP headers to carry UID/PID information
 * in the IP options - all other packets are ignored.
 * Also, see the comments above the individual steps taken in the code for details
 */

/* Metadata header structure */

struct knox_meta_param {
    uid_t uid;
    pid_t pid;
};

#define TUN_META_HDR_SZ sizeof(struct knox_meta_param)
#define TUN_META_MARK_OFFSET offsetof(struct knox_meta_param, uid)
// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }

#define FLT_EXACT_COUNT 8
struct tap_filter {
	unsigned int    count;    /* Number of addrs. Zero means disabled */
	u32             mask[2];  /* Mask of the hashed addrs */
	unsigned char	addr[FLT_EXACT_COUNT][ETH_ALEN];
};

/* MAX_TAP_QUEUES 256 is chosen to allow rx/tx queues to be equal
 * to max number of VCPUs in guest. */
#define MAX_TAP_QUEUES 256
#define MAX_TAP_FLOWS  4096

#define TUN_FLOW_EXPIRE (3 * HZ)

struct tun_pcpu_stats {
	u64 rx_packets;
	u64 rx_bytes;
	u64 tx_packets;
	u64 tx_bytes;
	struct u64_stats_sync syncp;
	u32 rx_dropped;
	u32 tx_dropped;
	u32 rx_frame_errors;
};

/* A tun_file connects an open character device to a tuntap netdevice. It
 * also contains all socket related structures (except sock_fprog and tap_filter)
 * to serve as one transmit queue for tuntap device. The sock_fprog and
 * tap_filter were kept in tun_struct since they were used for filtering for the
 * netdevice not for a specific queue (at least I didn't see the requirement for
 * this).
 *
 * RCU usage:
 * The tun_file and tun_struct are loosely coupled, the pointer from one to the
 * other can only be read while rcu_read_lock or rtnl_lock is held.
 */
struct tun_file {
	struct sock sk;
	struct socket socket;
	struct socket_wq wq;
	struct tun_struct __rcu *tun;
	struct fasync_struct *fasync;
	/* only used for fasnyc */
	unsigned int flags;
	union {
		u16 queue_index;
		unsigned int ifindex;
	};
	struct list_head next;
	struct tun_struct *detached;
	struct skb_array tx_array;
};

struct tun_flow_entry {
	struct hlist_node hash_link;
	struct rcu_head rcu;
	struct tun_struct *tun;

	u32 rxhash;
	u32 rps_rxhash;
	int queue_index;
	unsigned long updated;
};

#define TUN_NUM_FLOW_ENTRIES 1024

/* Since the socket were moved to tun_file, to preserve the behavior of persist
 * device, socket filter, sndbuf and vnet header size were restore when the
 * file were attached to a persist device.
 */
struct tun_struct {
	struct tun_file __rcu	*tfiles[MAX_TAP_QUEUES];
	unsigned int            numqueues;
	unsigned int 		flags;
	kuid_t			owner;
	kgid_t			group;

	struct net_device	*dev;
	netdev_features_t	set_features;
#define TUN_USER_FEATURES (NETIF_F_HW_CSUM|NETIF_F_TSO_ECN|NETIF_F_TSO| \
			  NETIF_F_TSO6)

	int			align;
	int			vnet_hdr_sz;
	int			sndbuf;
	struct tap_filter	txflt;
	struct sock_fprog	fprog;
	/* protected by rtnl lock */
	bool			filter_attached;
#ifdef TUN_DEBUG
	int debug;
#endif
	spinlock_t lock;
	struct hlist_head flows[TUN_NUM_FLOW_ENTRIES];
	struct timer_list flow_gc_timer;
	unsigned long ageing_time;
	unsigned int numdisabled;
	struct list_head disabled;
	void *security;
	u32 flow_count;
	u32 rx_batched;
	struct tun_pcpu_stats __percpu *pcpu_stats;
	struct bpf_prog __rcu *xdp_prog;
};

#ifdef CONFIG_TUN_VNET_CROSS_LE
static inline bool tun_legacy_is_little_endian(struct tun_struct *tun)
{
	return tun->flags & TUN_VNET_BE ? false :
		virtio_legacy_is_little_endian();
}

static long tun_get_vnet_be(struct tun_struct *tun, int __user *argp)
{
	int be = !!(tun->flags & TUN_VNET_BE);

	if (put_user(be, argp))
		return -EFAULT;

	return 0;
}

static long tun_set_vnet_be(struct tun_struct *tun, int __user *argp)
{
	int be;

	if (get_user(be, argp))
		return -EFAULT;

	if (be)
		tun->flags |= TUN_VNET_BE;
	else
		tun->flags &= ~TUN_VNET_BE;

	return 0;
}
#else
static inline bool tun_legacy_is_little_endian(struct tun_struct *tun)
{
	return virtio_legacy_is_little_endian();
}

static long tun_get_vnet_be(struct tun_struct *tun, int __user *argp)
{
	return -EINVAL;
}

static long tun_set_vnet_be(struct tun_struct *tun, int __user *argp)
{
	return -EINVAL;
}
#endif /* CONFIG_TUN_VNET_CROSS_LE */

static inline bool tun_is_little_endian(struct tun_struct *tun)
{
	return tun->flags & TUN_VNET_LE ||
		tun_legacy_is_little_endian(tun);
}

static inline u16 tun16_to_cpu(struct tun_struct *tun, __virtio16 val)
{
	return __virtio16_to_cpu(tun_is_little_endian(tun), val);
}

static inline __virtio16 cpu_to_tun16(struct tun_struct *tun, u16 val)
{
	return __cpu_to_virtio16(tun_is_little_endian(tun), val);
}

static inline u32 tun_hashfn(u32 rxhash)
{
	return rxhash & 0x3ff;
}

static struct tun_flow_entry *tun_flow_find(struct hlist_head *head, u32 rxhash)
{
	struct tun_flow_entry *e;

	hlist_for_each_entry_rcu(e, head, hash_link) {
		if (e->rxhash == rxhash)
			return e;
	}
	return NULL;
}

static struct tun_flow_entry *tun_flow_create(struct tun_struct *tun,
					      struct hlist_head *head,
					      u32 rxhash, u16 queue_index)
{
	struct tun_flow_entry *e = kmalloc(sizeof(*e), GFP_ATOMIC);

	if (e) {
		tun_debug(KERN_INFO, tun, "create flow: hash %u index %u\n",
			  rxhash, queue_index);
		e->updated = jiffies;
		e->rxhash = rxhash;
		e->rps_rxhash = 0;
		e->queue_index = queue_index;
		e->tun = tun;
		hlist_add_head_rcu(&e->hash_link, head);
		++tun->flow_count;
	}
	return e;
}

static void tun_flow_delete(struct tun_struct *tun, struct tun_flow_entry *e)
{
	tun_debug(KERN_INFO, tun, "delete flow: hash %u index %u\n",
		  e->rxhash, e->queue_index);
	hlist_del_rcu(&e->hash_link);
	kfree_rcu(e, rcu);
	--tun->flow_count;
}

static void tun_flow_flush(struct tun_struct *tun)
{
	int i;

	spin_lock_bh(&tun->lock);
	for (i = 0; i < TUN_NUM_FLOW_ENTRIES; i++) {
		struct tun_flow_entry *e;
		struct hlist_node *n;

		hlist_for_each_entry_safe(e, n, &tun->flows[i], hash_link)
			tun_flow_delete(tun, e);
	}
	spin_unlock_bh(&tun->lock);
}

static void tun_flow_delete_by_queue(struct tun_struct *tun, u16 queue_index)
{
	int i;

	spin_lock_bh(&tun->lock);
	for (i = 0; i < TUN_NUM_FLOW_ENTRIES; i++) {
		struct tun_flow_entry *e;
		struct hlist_node *n;

		hlist_for_each_entry_safe(e, n, &tun->flows[i], hash_link) {
			if (e->queue_index == queue_index)
				tun_flow_delete(tun, e);
		}
	}
	spin_unlock_bh(&tun->lock);
}

static void tun_flow_cleanup(unsigned long data)
{
	struct tun_struct *tun = (struct tun_struct *)data;
	unsigned long delay = tun->ageing_time;
	unsigned long next_timer = jiffies + delay;
	unsigned long count = 0;
	int i;

	tun_debug(KERN_INFO, tun, "tun_flow_cleanup\n");

	spin_lock_bh(&tun->lock);
	for (i = 0; i < TUN_NUM_FLOW_ENTRIES; i++) {
		struct tun_flow_entry *e;
		struct hlist_node *n;

		hlist_for_each_entry_safe(e, n, &tun->flows[i], hash_link) {
			unsigned long this_timer;
			count++;
			this_timer = e->updated + delay;
			if (time_before_eq(this_timer, jiffies))
				tun_flow_delete(tun, e);
			else if (time_before(this_timer, next_timer))
				next_timer = this_timer;
		}
	}

	if (count)
		mod_timer(&tun->flow_gc_timer, round_jiffies_up(next_timer));
	spin_unlock_bh(&tun->lock);
}

static void tun_flow_update(struct tun_struct *tun, u32 rxhash,
			    struct tun_file *tfile)
{
	struct hlist_head *head;
	struct tun_flow_entry *e;
	unsigned long delay = tun->ageing_time;
	u16 queue_index = tfile->queue_index;

	if (!rxhash)
		return;
	else
		head = &tun->flows[tun_hashfn(rxhash)];

	rcu_read_lock();

	/* We may get a very small possibility of OOO during switching, not
	 * worth to optimize.*/
	if (tun->numqueues == 1 || tfile->detached)
		goto unlock;

	e = tun_flow_find(head, rxhash);
	if (likely(e)) {
		/* TODO: keep queueing to old queue until it's empty? */
		e->queue_index = queue_index;
		e->updated = jiffies;
		sock_rps_record_flow_hash(e->rps_rxhash);
	} else {
		spin_lock_bh(&tun->lock);
		if (!tun_flow_find(head, rxhash) &&
		    tun->flow_count < MAX_TAP_FLOWS)
			tun_flow_create(tun, head, rxhash, queue_index);

		if (!timer_pending(&tun->flow_gc_timer))
			mod_timer(&tun->flow_gc_timer,
				  round_jiffies_up(jiffies + delay));
		spin_unlock_bh(&tun->lock);
	}

unlock:
	rcu_read_unlock();
}

/**
 * Save the hash received in the stack receive path and update the
 * flow_hash table accordingly.
 */
static inline void tun_flow_save_rps_rxhash(struct tun_flow_entry *e, u32 hash)
{
	if (unlikely(e->rps_rxhash != hash))
		e->rps_rxhash = hash;
}

/* We try to identify a flow through its rxhash first. The reason that
 * we do not check rxq no. is because some cards(e.g 82599), chooses
 * the rxq based on the txq where the last packet of the flow comes. As
 * the userspace application move between processors, we may get a
 * different rxq no. here. If we could not get rxhash, then we would
 * hope the rxq no. may help here.
 */
static u16 tun_select_queue(struct net_device *dev, struct sk_buff *skb,
			    void *accel_priv, select_queue_fallback_t fallback)
{
	struct tun_struct *tun = netdev_priv(dev);
	struct tun_flow_entry *e;
	u32 txq = 0;
	u32 numqueues = 0;

	rcu_read_lock();
	numqueues = ACCESS_ONCE(tun->numqueues);

	txq = __skb_get_hash_symmetric(skb);
	if (txq) {
		e = tun_flow_find(&tun->flows[tun_hashfn(txq)], txq);
		if (e) {
			tun_flow_save_rps_rxhash(e, txq);
			txq = e->queue_index;
		} else
			/* use multiply and shift instead of expensive divide */
			txq = ((u64)txq * numqueues) >> 32;
	} else if (likely(skb_rx_queue_recorded(skb))) {
		txq = skb_get_rx_queue(skb);
		while (unlikely(txq >= numqueues))
			txq -= numqueues;
	}

	rcu_read_unlock();
	return txq;
}

static inline bool tun_not_capable(struct tun_struct *tun)
{
	const struct cred *cred = current_cred();
	struct net *net = dev_net(tun->dev);

	return ((uid_valid(tun->owner) && !uid_eq(cred->euid, tun->owner)) ||
		  (gid_valid(tun->group) && !in_egroup_p(tun->group))) &&
		!ns_capable(net->user_ns, CAP_NET_ADMIN);
}

static void tun_set_real_num_queues(struct tun_struct *tun)
{
	netif_set_real_num_tx_queues(tun->dev, tun->numqueues);
	netif_set_real_num_rx_queues(tun->dev, tun->numqueues);
}

static void tun_disable_queue(struct tun_struct *tun, struct tun_file *tfile)
{
	tfile->detached = tun;
	list_add_tail(&tfile->next, &tun->disabled);
	++tun->numdisabled;
}

static struct tun_struct *tun_enable_queue(struct tun_file *tfile)
{
	struct tun_struct *tun = tfile->detached;

	tfile->detached = NULL;
	list_del_init(&tfile->next);
	--tun->numdisabled;
	return tun;
}

static void tun_queue_purge(struct tun_file *tfile)
{
	struct sk_buff *skb;

	while ((skb = skb_array_consume(&tfile->tx_array)) != NULL)
		kfree_skb(skb);

	skb_queue_purge(&tfile->sk.sk_write_queue);
	skb_queue_purge(&tfile->sk.sk_error_queue);
}

static void __tun_detach(struct tun_file *tfile, bool clean)
{
	struct tun_file *ntfile;
	struct tun_struct *tun;

	tun = rtnl_dereference(tfile->tun);

	if (tun && !tfile->detached) {
		u16 index = tfile->queue_index;
		BUG_ON(index >= tun->numqueues);

		rcu_assign_pointer(tun->tfiles[index],
				   tun->tfiles[tun->numqueues - 1]);
		ntfile = rtnl_dereference(tun->tfiles[index]);
		ntfile->queue_index = index;

		--tun->numqueues;
		if (clean) {
			RCU_INIT_POINTER(tfile->tun, NULL);
			sock_put(&tfile->sk);
		} else
			tun_disable_queue(tun, tfile);

		synchronize_net();
		tun_flow_delete_by_queue(tun, tun->numqueues + 1);
		/* Drop read queue */
		tun_queue_purge(tfile);
		tun_set_real_num_queues(tun);
	} else if (tfile->detached && clean) {
		tun = tun_enable_queue(tfile);
		sock_put(&tfile->sk);
	}

	if (clean) {
		if (tun && tun->numqueues == 0 && tun->numdisabled == 0) {
			netif_carrier_off(tun->dev);

			if (!(tun->flags & IFF_PERSIST) &&
			    tun->dev->reg_state == NETREG_REGISTERED)
				unregister_netdevice(tun->dev);
		}
		skb_array_cleanup(&tfile->tx_array);
		sock_put(&tfile->sk);
	}
}

static void tun_detach(struct tun_file *tfile, bool clean)
{
	rtnl_lock();
	__tun_detach(tfile, clean);
	rtnl_unlock();
}

static void tun_detach_all(struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);
	struct bpf_prog *xdp_prog = rtnl_dereference(tun->xdp_prog);
	struct tun_file *tfile, *tmp;
	int i, n = tun->numqueues;

	for (i = 0; i < n; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		BUG_ON(!tfile);
		tfile->socket.sk->sk_shutdown = RCV_SHUTDOWN;
		tfile->socket.sk->sk_data_ready(tfile->socket.sk);
		RCU_INIT_POINTER(tfile->tun, NULL);
		--tun->numqueues;
	}
	list_for_each_entry(tfile, &tun->disabled, next) {
		tfile->socket.sk->sk_shutdown = RCV_SHUTDOWN;
		tfile->socket.sk->sk_data_ready(tfile->socket.sk);
		RCU_INIT_POINTER(tfile->tun, NULL);
	}
	BUG_ON(tun->numqueues != 0);

	synchronize_net();
	for (i = 0; i < n; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		/* Drop read queue */
		tun_queue_purge(tfile);
		sock_put(&tfile->sk);
	}
	list_for_each_entry_safe(tfile, tmp, &tun->disabled, next) {
		tun_enable_queue(tfile);
		tun_queue_purge(tfile);
		sock_put(&tfile->sk);
	}
	BUG_ON(tun->numdisabled != 0);

	if (xdp_prog)
		bpf_prog_put(xdp_prog);

	if (tun->flags & IFF_PERSIST)
		module_put(THIS_MODULE);
}

static int tun_attach(struct tun_struct *tun, struct file *file,
		      bool skip_filter, bool publish_tun)
{
	struct tun_file *tfile = file->private_data;
	struct net_device *dev = tun->dev;
	int err;

	err = security_tun_dev_attach(tfile->socket.sk, tun->security);
	if (err < 0)
		goto out;

	err = -EINVAL;
	if (rtnl_dereference(tfile->tun) && !tfile->detached)
		goto out;

	err = -EBUSY;
	if (!(tun->flags & IFF_MULTI_QUEUE) && tun->numqueues == 1)
		goto out;

	err = -E2BIG;
	if (!tfile->detached &&
	    tun->numqueues + tun->numdisabled == MAX_TAP_QUEUES)
		goto out;

	err = 0;

	/* Re-attach the filter to persist device */
	if (!skip_filter && (tun->filter_attached == true)) {
		lock_sock(tfile->socket.sk);
		err = sk_attach_filter(&tun->fprog, tfile->socket.sk);
		release_sock(tfile->socket.sk);
		if (!err)
			goto out;
	}

	if (!tfile->detached &&
	    skb_array_resize(&tfile->tx_array, dev->tx_queue_len, GFP_KERNEL)) {
		err = -ENOMEM;
		goto out;
	}

	tfile->queue_index = tun->numqueues;
	tfile->socket.sk->sk_shutdown &= ~RCV_SHUTDOWN;
	if (publish_tun)
		rcu_assign_pointer(tfile->tun, tun);
	rcu_assign_pointer(tun->tfiles[tun->numqueues], tfile);
	tun->numqueues++;

	if (tfile->detached)
		tun_enable_queue(tfile);
	else
		sock_hold(&tfile->sk);

	tun_set_real_num_queues(tun);

	/* device is allowed to go away first, so no need to hold extra
	 * refcnt.
	 */

out:
	return err;
}

static struct tun_struct *__tun_get(struct tun_file *tfile)
{
	struct tun_struct *tun;

	rcu_read_lock();
	tun = rcu_dereference(tfile->tun);
	if (tun)
		dev_hold(tun->dev);
	rcu_read_unlock();

	return tun;
}

static struct tun_struct *tun_get(struct file *file)
{
	return __tun_get(file->private_data);
}

static void tun_put(struct tun_struct *tun)
{
	dev_put(tun->dev);
}

/* TAP filtering */
static void addr_hash_set(u32 *mask, const u8 *addr)
{
	int n = ether_crc(ETH_ALEN, addr) >> 26;
	mask[n >> 5] |= (1 << (n & 31));
}

static unsigned int addr_hash_test(const u32 *mask, const u8 *addr)
{
	int n = ether_crc(ETH_ALEN, addr) >> 26;
	return mask[n >> 5] & (1 << (n & 31));
}

static int update_filter(struct tap_filter *filter, void __user *arg)
{
	struct { u8 u[ETH_ALEN]; } *addr;
	struct tun_filter uf;
	int err, alen, n, nexact;

	if (copy_from_user(&uf, arg, sizeof(uf)))
		return -EFAULT;

	if (!uf.count) {
		/* Disabled */
		filter->count = 0;
		return 0;
	}

	alen = ETH_ALEN * uf.count;
	addr = memdup_user(arg + sizeof(uf), alen);
	if (IS_ERR(addr))
		return PTR_ERR(addr);

	/* The filter is updated without holding any locks. Which is
	 * perfectly safe. We disable it first and in the worst
	 * case we'll accept a few undesired packets. */
	filter->count = 0;
	wmb();

	/* Use first set of addresses as an exact filter */
	for (n = 0; n < uf.count && n < FLT_EXACT_COUNT; n++)
		memcpy(filter->addr[n], addr[n].u, ETH_ALEN);

	nexact = n;

	/* Remaining multicast addresses are hashed,
	 * unicast will leave the filter disabled. */
	memset(filter->mask, 0, sizeof(filter->mask));
	for (; n < uf.count; n++) {
		if (!is_multicast_ether_addr(addr[n].u)) {
			err = 0; /* no filter */
			goto free_addr;
		}
		addr_hash_set(filter->mask, addr[n].u);
	}

	/* For ALLMULTI just set the mask to all ones.
	 * This overrides the mask populated above. */
	if ((uf.flags & TUN_FLT_ALLMULTI))
		memset(filter->mask, ~0, sizeof(filter->mask));

	/* Now enable the filter */
	wmb();
	filter->count = nexact;

	/* Return the number of exact filters */
	err = nexact;
free_addr:
	kfree(addr);
	return err;
}

/* Returns: 0 - drop, !=0 - accept */
static int run_filter(struct tap_filter *filter, const struct sk_buff *skb)
{
	/* Cannot use eth_hdr(skb) here because skb_mac_hdr() is incorrect
	 * at this point. */
	struct ethhdr *eh = (struct ethhdr *) skb->data;
	int i;

	/* Exact match */
	for (i = 0; i < filter->count; i++)
		if (ether_addr_equal(eh->h_dest, filter->addr[i]))
			return 1;

	/* Inexact match (multicast only) */
	if (is_multicast_ether_addr(eh->h_dest))
		return addr_hash_test(filter->mask, eh->h_dest);

	return 0;
}

/*
 * Checks whether the packet is accepted or not.
 * Returns: 0 - drop, !=0 - accept
 */
static int check_filter(struct tap_filter *filter, const struct sk_buff *skb)
{
	if (!filter->count)
		return 1;

	return run_filter(filter, skb);
}

/* Network device part of the driver */

static const struct ethtool_ops tun_ethtool_ops;

/* Net device detach from fd. */
static void tun_net_uninit(struct net_device *dev)
{
	tun_detach_all(dev);
}

/* Net device open. */
static int tun_net_open(struct net_device *dev)
{
	netif_tx_start_all_queues(dev);

	return 0;
}

/* Net device close. */
static int tun_net_close(struct net_device *dev)
{
	netif_tx_stop_all_queues(dev);
	return 0;
}

/* Net device start xmit */
static netdev_tx_t tun_net_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);
	int txq = skb->queue_mapping;
	struct netdev_queue *queue;
	struct tun_file *tfile;
	u32 numqueues = 0;

	rcu_read_lock();
	tfile = rcu_dereference(tun->tfiles[txq]);
	numqueues = ACCESS_ONCE(tun->numqueues);

	/* Drop packet if interface is not attached */
	if (txq >= numqueues)
		goto drop;

#ifdef CONFIG_RPS
	if (numqueues == 1 && static_key_false(&rps_needed)) {
		/* Select queue was not called for the skbuff, so we extract the
		 * RPS hash and save it into the flow_table here.
		 */
		__u32 rxhash;

		rxhash = __skb_get_hash_symmetric(skb);
		if (rxhash) {
			struct tun_flow_entry *e;
			e = tun_flow_find(&tun->flows[tun_hashfn(rxhash)],
					rxhash);
			if (e)
				tun_flow_save_rps_rxhash(e, rxhash);
		}
	}
#endif

	tun_debug(KERN_INFO, tun, "tun_net_xmit %d\n", skb->len);

	BUG_ON(!tfile);

	/* Drop if the filter does not like it.
	 * This is a noop if the filter is disabled.
	 * Filter can be enabled only for the TAP devices. */
	if (!check_filter(&tun->txflt, skb))
		goto drop;

	if (tfile->socket.sk->sk_filter &&
	    sk_filter(tfile->socket.sk, skb))
		goto drop;

	if (unlikely(skb_orphan_frags_rx(skb, GFP_ATOMIC)))
		goto drop;

	skb_tx_timestamp(skb);

	/* Orphan the skb - required as we might hang on to it
	 * for indefinite time.
	 */
	skb_orphan(skb);

	nf_reset(skb);

	if (skb_array_produce(&tfile->tx_array, skb))
		goto drop;

	/* NETIF_F_LLTX requires to do our own update of trans_start */
	queue = netdev_get_tx_queue(dev, txq);
	queue->trans_start = jiffies;

	/* Notify and wake up reader process */
	if (tfile->flags & TUN_FASYNC)
		kill_fasync(&tfile->fasync, SIGIO, POLL_IN);
	tfile->socket.sk->sk_data_ready(tfile->socket.sk);

	rcu_read_unlock();
	return NETDEV_TX_OK;

drop:
	this_cpu_inc(tun->pcpu_stats->tx_dropped);
	skb_tx_error(skb);
	kfree_skb(skb);
	rcu_read_unlock();
	return NET_XMIT_DROP;
}

static void tun_net_mclist(struct net_device *dev)
{
	/*
	 * This callback is supposed to deal with mc filter in
	 * _rx_ path and has nothing to do with the _tx_ path.
	 * In rx path we always accept everything userspace gives us.
	 */
}

static netdev_features_t tun_net_fix_features(struct net_device *dev,
	netdev_features_t features)
{
	struct tun_struct *tun = netdev_priv(dev);

	return (features & tun->set_features) | (features & ~TUN_USER_FEATURES);
}
#ifdef CONFIG_NET_POLL_CONTROLLER
static void tun_poll_controller(struct net_device *dev)
{
	/*
	 * Tun only receives frames when:
	 * 1) the char device endpoint gets data from user space
	 * 2) the tun socket gets a sendmsg call from user space
	 * Since both of those are synchronous operations, we are guaranteed
	 * never to have pending data when we poll for it
	 * so there is nothing to do here but return.
	 * We need this though so netpoll recognizes us as an interface that
	 * supports polling, which enables bridge devices in virt setups to
	 * still use netconsole
	 */
	return;
}
#endif

static void tun_set_headroom(struct net_device *dev, int new_hr)
{
	struct tun_struct *tun = netdev_priv(dev);

	if (new_hr < NET_SKB_PAD)
		new_hr = NET_SKB_PAD;

	tun->align = new_hr;
}

static void
tun_net_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	u32 rx_dropped = 0, tx_dropped = 0, rx_frame_errors = 0;
	struct tun_struct *tun = netdev_priv(dev);
	struct tun_pcpu_stats *p;
	int i;

	for_each_possible_cpu(i) {
		u64 rxpackets, rxbytes, txpackets, txbytes;
		unsigned int start;

		p = per_cpu_ptr(tun->pcpu_stats, i);
		do {
			start = u64_stats_fetch_begin(&p->syncp);
			rxpackets	= p->rx_packets;
			rxbytes		= p->rx_bytes;
			txpackets	= p->tx_packets;
			txbytes		= p->tx_bytes;
		} while (u64_stats_fetch_retry(&p->syncp, start));

		stats->rx_packets	+= rxpackets;
		stats->rx_bytes		+= rxbytes;
		stats->tx_packets	+= txpackets;
		stats->tx_bytes		+= txbytes;

		/* u32 counters */
		rx_dropped	+= p->rx_dropped;
		rx_frame_errors	+= p->rx_frame_errors;
		tx_dropped	+= p->tx_dropped;
	}
	stats->rx_dropped  = rx_dropped;
	stats->rx_frame_errors = rx_frame_errors;
	stats->tx_dropped = tx_dropped;
}

static int tun_xdp_set(struct net_device *dev, struct bpf_prog *prog,
		       struct netlink_ext_ack *extack)
{
	struct tun_struct *tun = netdev_priv(dev);
	struct bpf_prog *old_prog;

	old_prog = rtnl_dereference(tun->xdp_prog);
	rcu_assign_pointer(tun->xdp_prog, prog);
	if (old_prog)
		bpf_prog_put(old_prog);

	return 0;
}

static u32 tun_xdp_query(struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);
	const struct bpf_prog *xdp_prog;

	xdp_prog = rtnl_dereference(tun->xdp_prog);
	if (xdp_prog)
		return xdp_prog->aux->id;

	return 0;
}

static int tun_xdp(struct net_device *dev, struct netdev_bpf *xdp)
{
	switch (xdp->command) {
	case XDP_SETUP_PROG:
		return tun_xdp_set(dev, xdp->prog, xdp->extack);
	case XDP_QUERY_PROG:
		xdp->prog_id = tun_xdp_query(dev);
		xdp->prog_attached = !!xdp->prog_id;
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct net_device_ops tun_netdev_ops = {
	.ndo_uninit		= tun_net_uninit,
	.ndo_open		= tun_net_open,
	.ndo_stop		= tun_net_close,
	.ndo_start_xmit		= tun_net_xmit,
	.ndo_fix_features	= tun_net_fix_features,
	.ndo_select_queue	= tun_select_queue,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= tun_poll_controller,
#endif
	.ndo_set_rx_headroom	= tun_set_headroom,
	.ndo_get_stats64	= tun_net_get_stats64,
};

static const struct net_device_ops tap_netdev_ops = {
	.ndo_uninit		= tun_net_uninit,
	.ndo_open		= tun_net_open,
	.ndo_stop		= tun_net_close,
	.ndo_start_xmit		= tun_net_xmit,
	.ndo_fix_features	= tun_net_fix_features,
	.ndo_set_rx_mode	= tun_net_mclist,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_select_queue	= tun_select_queue,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= tun_poll_controller,
#endif
	.ndo_features_check	= passthru_features_check,
	.ndo_set_rx_headroom	= tun_set_headroom,
	.ndo_get_stats64	= tun_net_get_stats64,
	.ndo_bpf		= tun_xdp,
};

static void tun_flow_init(struct tun_struct *tun)
{
	int i;

	for (i = 0; i < TUN_NUM_FLOW_ENTRIES; i++)
		INIT_HLIST_HEAD(&tun->flows[i]);

	tun->ageing_time = TUN_FLOW_EXPIRE;
	setup_timer(&tun->flow_gc_timer, tun_flow_cleanup, (unsigned long)tun);
	mod_timer(&tun->flow_gc_timer,
		  round_jiffies_up(jiffies + tun->ageing_time));
}

static void tun_flow_uninit(struct tun_struct *tun)
{
	del_timer_sync(&tun->flow_gc_timer);
	tun_flow_flush(tun);
}

#define MIN_MTU 68
#define MAX_MTU 65535

/* Initialize net device. */
static void tun_net_init(struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);

	switch (tun->flags & TUN_TYPE_MASK) {
	case IFF_TUN:
		dev->netdev_ops = &tun_netdev_ops;

		/* Point-to-Point TUN Device */
		dev->hard_header_len = 0;
		dev->addr_len = 0;
		dev->mtu = 1500;

		/* Zero header length */
		dev->type = ARPHRD_NONE;
		dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
		break;

	case IFF_TAP:
		dev->netdev_ops = &tap_netdev_ops;
		/* Ethernet TAP Device */
		ether_setup(dev);
		dev->priv_flags &= ~IFF_TX_SKB_SHARING;
		dev->priv_flags |= IFF_LIVE_ADDR_CHANGE;

		eth_hw_addr_random(dev);

		break;
	}

	dev->min_mtu = MIN_MTU;
	dev->max_mtu = MAX_MTU - dev->hard_header_len;
}

static bool tun_sock_writeable(struct tun_struct *tun, struct tun_file *tfile)
{
	struct sock *sk = tfile->socket.sk;

	return (tun->dev->flags & IFF_UP) && sock_writeable(sk);
}

/* Character device part */

/* Poll */
static unsigned int tun_chr_poll(struct file *file, poll_table *wait)
{
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun = __tun_get(tfile);
	struct sock *sk;
	unsigned int mask = 0;

	if (!tun)
		return POLLERR;

	sk = tfile->socket.sk;

	tun_debug(KERN_INFO, tun, "tun_chr_poll\n");

	poll_wait(file, sk_sleep(sk), wait);

	if (!skb_array_empty(&tfile->tx_array))
		mask |= POLLIN | POLLRDNORM;

	/* Make sure SOCKWQ_ASYNC_NOSPACE is set if not writable to
	 * guarantee EPOLLOUT to be raised by either here or
	 * tun_sock_write_space(). Then process could get notification
	 * after it writes to a down device and meets -EIO.
	 */
	if (tun_sock_writeable(tun, tfile) ||
	    (!test_and_set_bit(SOCKWQ_ASYNC_NOSPACE, &sk->sk_socket->flags) &&
	     tun_sock_writeable(tun, tfile)))
		mask |= POLLOUT | POLLWRNORM;

	if (tun->dev->reg_state != NETREG_REGISTERED)
		mask = POLLERR;

	tun_put(tun);
	return mask;
}

/* prepad is the amount to reserve at front.  len is length after that.
 * linear is a hint as to how much to copy (usually headers). */
static struct sk_buff *tun_alloc_skb(struct tun_file *tfile,
				     size_t prepad, size_t len,
				     size_t linear, int noblock)
{
	struct sock *sk = tfile->socket.sk;
	struct sk_buff *skb;
	int err;

	/* Under a page?  Don't bother with paged skb. */
	if (prepad + len < PAGE_SIZE || !linear)
		linear = len;

	skb = sock_alloc_send_pskb(sk, prepad + linear, len - linear, noblock,
				   &err, 0);
	if (!skb)
		return ERR_PTR(err);

	skb_reserve(skb, prepad);
	skb_put(skb, linear);
	skb->data_len = len - linear;
	skb->len += len - linear;

	return skb;
}

static void tun_rx_batched(struct tun_struct *tun, struct tun_file *tfile,
			   struct sk_buff *skb, int more)
{
	struct sk_buff_head *queue = &tfile->sk.sk_write_queue;
	struct sk_buff_head process_queue;
	u32 rx_batched = tun->rx_batched;
	bool rcv = false;

	if (!rx_batched || (!more && skb_queue_empty(queue))) {
		local_bh_disable();
		skb_record_rx_queue(skb, tfile->queue_index);
		netif_receive_skb(skb);
		local_bh_enable();
		return;
	}

	spin_lock(&queue->lock);
	if (!more || skb_queue_len(queue) == rx_batched) {
		__skb_queue_head_init(&process_queue);
		skb_queue_splice_tail_init(queue, &process_queue);
		rcv = true;
	} else {
		__skb_queue_tail(queue, skb);
	}
	spin_unlock(&queue->lock);

	if (rcv) {
		struct sk_buff *nskb;

		local_bh_disable();
		while ((nskb = __skb_dequeue(&process_queue))) {
			skb_record_rx_queue(nskb, tfile->queue_index);
			netif_receive_skb(nskb);
		}
		skb_record_rx_queue(skb, tfile->queue_index);
		netif_receive_skb(skb);
		local_bh_enable();
	}
}

static bool tun_can_build_skb(struct tun_struct *tun, struct tun_file *tfile,
			      int len, int noblock, bool zerocopy)
{
	if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
		return false;

	if (tfile->socket.sk->sk_sndbuf != INT_MAX)
		return false;

	if (!noblock)
		return false;

	if (zerocopy)
		return false;

	if (SKB_DATA_ALIGN(len + TUN_RX_PAD + XDP_PACKET_HEADROOM) +
	    SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) > PAGE_SIZE)
		return false;

	return true;
}

static struct sk_buff *tun_build_skb(struct tun_struct *tun,
				     struct tun_file *tfile,
				     struct iov_iter *from,
				     struct virtio_net_hdr *hdr,
				     int len, int *skb_xdp)
{
	struct page_frag *alloc_frag = &current->task_frag;
	struct sk_buff *skb;
	struct bpf_prog *xdp_prog;
	int buflen = SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
	unsigned int delta = 0;
	char *buf;
	size_t copied;
	bool xdp_xmit = false;
	int err, pad = TUN_RX_PAD;

	rcu_read_lock();
	xdp_prog = rcu_dereference(tun->xdp_prog);
	if (xdp_prog)
		pad += TUN_HEADROOM;
	buflen += SKB_DATA_ALIGN(len + pad);
	rcu_read_unlock();

	alloc_frag->offset = ALIGN((u64)alloc_frag->offset, SMP_CACHE_BYTES);
	if (unlikely(!skb_page_frag_refill(buflen, alloc_frag, GFP_KERNEL)))
		return ERR_PTR(-ENOMEM);

	buf = (char *)page_address(alloc_frag->page) + alloc_frag->offset;
	copied = copy_page_from_iter(alloc_frag->page,
				     alloc_frag->offset + pad,
				     len, from);
	if (copied != len)
		return ERR_PTR(-EFAULT);

	/* There's a small window that XDP may be set after the check
	 * of xdp_prog above, this should be rare and for simplicity
	 * we do XDP on skb in case the headroom is not enough.
	 */
	if (hdr->gso_type || !xdp_prog)
		*skb_xdp = 1;
	else
		*skb_xdp = 0;

	local_bh_disable();
	rcu_read_lock();
	xdp_prog = rcu_dereference(tun->xdp_prog);
	if (xdp_prog && !*skb_xdp) {
		struct xdp_buff xdp;
		void *orig_data;
		u32 act;

		xdp.data_hard_start = buf;
		xdp.data = buf + pad;
		xdp_set_data_meta_invalid(&xdp);
		xdp.data_end = xdp.data + len;
		orig_data = xdp.data;
		act = bpf_prog_run_xdp(xdp_prog, &xdp);

		switch (act) {
		case XDP_REDIRECT:
			get_page(alloc_frag->page);
			alloc_frag->offset += buflen;
			err = xdp_do_redirect(tun->dev, &xdp, xdp_prog);
			xdp_do_flush_map();
			if (err)
				goto err_redirect;
			rcu_read_unlock();
			local_bh_enable();
			return NULL;
		case XDP_TX:
			xdp_xmit = true;
			/* fall through */
		case XDP_PASS:
			delta = orig_data - xdp.data;
			break;
		default:
			bpf_warn_invalid_xdp_action(act);
			/* fall through */
		case XDP_ABORTED:
			trace_xdp_exception(tun->dev, xdp_prog, act);
			/* fall through */
		case XDP_DROP:
			goto err_xdp;
		}
	}

	skb = build_skb(buf, buflen);
	if (!skb) {
		rcu_read_unlock();
		local_bh_enable();
		return ERR_PTR(-ENOMEM);
	}

	skb_reserve(skb, pad - delta);
	skb_put(skb, len + delta);
	skb_set_owner_w(skb, tfile->socket.sk);
	get_page(alloc_frag->page);
	alloc_frag->offset += buflen;

	if (xdp_xmit) {
		skb->dev = tun->dev;
		generic_xdp_tx(skb, xdp_prog);
		rcu_read_unlock();
		local_bh_enable();
		return NULL;
	}

	rcu_read_unlock();
	local_bh_enable();

	return skb;

err_redirect:
	put_page(alloc_frag->page);
err_xdp:
	rcu_read_unlock();
	local_bh_enable();
	this_cpu_inc(tun->pcpu_stats->rx_dropped);
	return NULL;
}

/* Get packet from user space buffer */
static ssize_t tun_get_user(struct tun_struct *tun, struct tun_file *tfile,
			    void *msg_control, struct iov_iter *from,
			    int noblock, bool more)
{
	struct tun_pi pi = { 0, cpu_to_be16(ETH_P_IP) };
	struct sk_buff *skb;
	size_t total_len = iov_iter_count(from);
	size_t len = total_len, align = tun->align, linear;
	struct virtio_net_hdr gso = { 0 };
	struct tun_pcpu_stats *stats;
	int good_linear;
	int copylen;
	bool zerocopy = false;
	int err;
	u32 rxhash;
	int skb_xdp = 1;

	if (!(tun->flags & IFF_NO_PI)) {
		if (len < sizeof(pi))
			return -EINVAL;
		len -= sizeof(pi);

		if (!copy_from_iter_full(&pi, sizeof(pi), from))
			return -EFAULT;
	}

	if (tun->flags & IFF_VNET_HDR) {
		int vnet_hdr_sz = READ_ONCE(tun->vnet_hdr_sz);

		if (len < vnet_hdr_sz)
			return -EINVAL;
		len -= vnet_hdr_sz;

		if (!copy_from_iter_full(&gso, sizeof(gso), from))
			return -EFAULT;

		if ((gso.flags & VIRTIO_NET_HDR_F_NEEDS_CSUM) &&
		    tun16_to_cpu(tun, gso.csum_start) + tun16_to_cpu(tun, gso.csum_offset) + 2 > tun16_to_cpu(tun, gso.hdr_len))
			gso.hdr_len = cpu_to_tun16(tun, tun16_to_cpu(tun, gso.csum_start) + tun16_to_cpu(tun, gso.csum_offset) + 2);

		if (tun16_to_cpu(tun, gso.hdr_len) > len)
			return -EINVAL;
		iov_iter_advance(from, vnet_hdr_sz - sizeof(gso));
	}

	if ((tun->flags & TUN_TYPE_MASK) == IFF_TAP) {
		align += NET_IP_ALIGN;
		if (unlikely(len < ETH_HLEN ||
			     (gso.hdr_len && tun16_to_cpu(tun, gso.hdr_len) < ETH_HLEN)))
			return -EINVAL;
	}

	good_linear = SKB_MAX_HEAD(align);

	if (msg_control) {
		struct iov_iter i = *from;

		/* There are 256 bytes to be copied in skb, so there is
		 * enough room for skb expand head in case it is used.
		 * The rest of the buffer is mapped from userspace.
		 */
		copylen = gso.hdr_len ? tun16_to_cpu(tun, gso.hdr_len) : GOODCOPY_LEN;
		if (copylen > good_linear)
			copylen = good_linear;
		linear = copylen;
		iov_iter_advance(&i, copylen);
		if (iov_iter_npages(&i, INT_MAX) <= MAX_SKB_FRAGS)
			zerocopy = true;
	}

	if (tun_can_build_skb(tun, tfile, len, noblock, zerocopy)) {
		/* For the packet that is not easy to be processed
		 * (e.g gso or jumbo packet), we will do it at after
		 * skb was created with generic XDP routine.
		 */
		skb = tun_build_skb(tun, tfile, from, &gso, len, &skb_xdp);
		if (IS_ERR(skb)) {
			this_cpu_inc(tun->pcpu_stats->rx_dropped);
			return PTR_ERR(skb);
		}
		if (!skb)
			return total_len;
	} else {
		if (!zerocopy) {
			copylen = len;
			if (tun16_to_cpu(tun, gso.hdr_len) > good_linear)
				linear = good_linear;
			else
				linear = tun16_to_cpu(tun, gso.hdr_len);
		}

		skb = tun_alloc_skb(tfile, align, copylen, linear, noblock);
		if (IS_ERR(skb)) {
			if (PTR_ERR(skb) != -EAGAIN)
				this_cpu_inc(tun->pcpu_stats->rx_dropped);
			return PTR_ERR(skb);
		}

		if (zerocopy)
			err = zerocopy_sg_from_iter(skb, from);
		else
			err = skb_copy_datagram_from_iter(skb, 0, from, len);

		if (err) {
			err = -EFAULT;
drop:
			this_cpu_inc(tun->pcpu_stats->rx_dropped);
			kfree_skb(skb);
			return err;
		}
	}

	if (virtio_net_hdr_to_skb(skb, &gso, tun_is_little_endian(tun))) {
		this_cpu_inc(tun->pcpu_stats->rx_frame_errors);
		kfree_skb(skb);
		return -EINVAL;
	}

	switch (tun->flags & TUN_TYPE_MASK) {
	case IFF_TUN:
		if (tun->flags & IFF_NO_PI) {
			u8 ip_version = skb->len ? (skb->data[0] >> 4) : 0;

			switch (ip_version) {
			case 4:
				pi.proto = htons(ETH_P_IP);
				break;
			case 6:
				pi.proto = htons(ETH_P_IPV6);
				break;
			default:
				this_cpu_inc(tun->pcpu_stats->rx_dropped);
				kfree_skb(skb);
				return -EINVAL;
			}
		}

		skb_reset_mac_header(skb);
		skb->protocol = pi.proto;
		skb->dev = tun->dev;
		break;
	case IFF_TAP:
		skb->protocol = eth_type_trans(skb, tun->dev);
		break;
	}

	/* copy skb_ubuf_info for callback when skb has no error */
	if (zerocopy) {
		skb_shinfo(skb)->destructor_arg = msg_control;
		skb_shinfo(skb)->tx_flags |= SKBTX_DEV_ZEROCOPY;
		skb_shinfo(skb)->tx_flags |= SKBTX_SHARED_FRAG;
	} else if (msg_control) {
		struct ubuf_info *uarg = msg_control;
		uarg->callback(uarg, false);
	}

	skb_reset_network_header(skb);
	skb_probe_transport_header(skb, 0);

	if (skb_xdp) {
		struct bpf_prog *xdp_prog;
		int ret;

		local_bh_disable();
		rcu_read_lock();
		xdp_prog = rcu_dereference(tun->xdp_prog);
		if (xdp_prog) {
			ret = do_xdp_generic(xdp_prog, skb);
			if (ret != XDP_PASS) {
				rcu_read_unlock();
				local_bh_enable();
				return total_len;
			}
		}
		rcu_read_unlock();
		local_bh_enable();
	}

	rxhash = __skb_get_hash_symmetric(skb);

	rcu_read_lock();
	if (unlikely(!(tun->dev->flags & IFF_UP))) {
		err = -EIO;
		rcu_read_unlock();
		goto drop;
	}

#ifndef CONFIG_4KSTACKS
	tun_rx_batched(tun, tfile, skb, more);
#else
	netif_rx_ni(skb);
#endif
	rcu_read_unlock();

	stats = get_cpu_ptr(tun->pcpu_stats);
	u64_stats_update_begin(&stats->syncp);
	stats->rx_packets++;
	stats->rx_bytes += len;
	u64_stats_update_end(&stats->syncp);
	put_cpu_ptr(stats);

	tun_flow_update(tun, rxhash, tfile);
	return total_len;
}

static ssize_t tun_chr_write_iter(struct kiocb *iocb, struct iov_iter *from)
{
	struct file *file = iocb->ki_filp;
	struct tun_struct *tun = tun_get(file);
	struct tun_file *tfile = file->private_data;
	ssize_t result;
	int noblock = 0;

	if (!tun)
		return -EBADFD;

	if ((file->f_flags & O_NONBLOCK) || (iocb->ki_flags & IOCB_NOWAIT))
		noblock = 1;

	result = tun_get_user(tun, tfile, NULL, from, noblock, false);

	tun_put(tun);
	return result;
}

// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
static int get_meta_param_values(struct sk_buff *skb, struct knox_meta_param *metalocal) {

	struct skb_shared_info *knox_shinfo = NULL;

	if (skb != NULL)
		knox_shinfo = skb_shinfo(skb);
	else {
#ifdef TUN_DEBUG
		pr_err("KNOX: NULL SKB in knoxvpn_process_uidpid");
#endif
		return 1;
	}

	if (knox_shinfo == NULL) {
#ifdef TUN_DEBUG
		pr_err("KNOX: knox_shinfo value is null");
#endif
		return 1;
	}

	if (knox_shinfo->knox_mark >= META_MARK_BASE_LOWER && knox_shinfo->knox_mark <= META_MARK_BASE_UPPER) {
		metalocal->uid = knox_shinfo->uid;
		metalocal->pid = knox_shinfo->pid;
	}

	if (knox_shinfo != NULL) {
		knox_shinfo->uid = knox_shinfo->pid = 0;
		knox_shinfo->knox_mark = 0;
	}

	return 0;
}
// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }

/* Put packet to the user space buffer */
static ssize_t tun_put_user(struct tun_struct *tun,
			    struct tun_file *tfile,
			    struct sk_buff *skb,
			    struct iov_iter *iter)
{
	struct tun_pi pi = { 0, skb->protocol };
// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
	struct knox_meta_param metalocal = { 0, 0 };
	int meta_param_get_status = 0;
// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }
	struct tun_pcpu_stats *stats;
	ssize_t total;
	int vlan_offset = 0;
	int vlan_hlen = 0;
	int vnet_hdr_sz = 0;

	if (skb_vlan_tag_present(skb))
		vlan_hlen = VLAN_HLEN;

	if (tun->flags & IFF_VNET_HDR)
		vnet_hdr_sz = READ_ONCE(tun->vnet_hdr_sz);

	total = skb->len + vlan_hlen + vnet_hdr_sz;

	if (!(tun->flags & IFF_NO_PI)) {
		if (iov_iter_count(iter) < sizeof(pi))
			return -EINVAL;

		total += sizeof(pi);
		if (iov_iter_count(iter) < total) {
			/* Packet will be striped */
			pi.flags |= TUN_PKT_STRIP;
		}

		if (copy_to_iter(&pi, sizeof(pi), iter) != sizeof(pi))
			return -EFAULT;
	}

// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
	meta_param_get_status = get_meta_param_values(skb, &metalocal);
	if (meta_param_get_status == 1) {
#ifdef TUN_DEBUG
		pr_err("KNOX: Error obtaining meta param values");
#endif
	} else {

		if (tun->flags & TUN_META_HDR) {
#ifdef TUN_DEBUG
			pr_err("KNOX: Appending uid: %d and pid: %d", metalocal.uid,
			       metalocal.pid);
#endif
			if (iov_iter_count(iter) < sizeof(struct knox_meta_param)) {
				return -EINVAL;
			}

			total += sizeof(struct knox_meta_param);
			if (copy_to_iter(&metalocal, sizeof(struct knox_meta_param), iter) != sizeof(struct knox_meta_param))
				return -EFAULT;
		}
	}
// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }
	if (vnet_hdr_sz) {
		struct virtio_net_hdr gso;

		if (iov_iter_count(iter) < vnet_hdr_sz)
			return -EINVAL;

		if (virtio_net_hdr_from_skb(skb, &gso,
					    tun_is_little_endian(tun), true,
					    vlan_hlen)) {
			struct skb_shared_info *sinfo = skb_shinfo(skb);

			if (net_ratelimit()) {
				netdev_err(tun->dev, "unexpected GSO type: 0x%x, gso_size %d, hdr_len %d\n",
					   sinfo->gso_type, tun16_to_cpu(tun, gso.gso_size),
					   tun16_to_cpu(tun, gso.hdr_len));
				print_hex_dump(KERN_ERR, "tun: ",
					       DUMP_PREFIX_NONE,
					       16, 1, skb->head,
					       min((int)tun16_to_cpu(tun, gso.hdr_len), 64), true);
			}
			WARN_ON_ONCE(1);
			return -EINVAL;
		}

		if (copy_to_iter(&gso, sizeof(gso), iter) != sizeof(gso))
			return -EFAULT;

		iov_iter_advance(iter, vnet_hdr_sz - sizeof(gso));
	}

	if (vlan_hlen) {
		int ret;
		struct {
			__be16 h_vlan_proto;
			__be16 h_vlan_TCI;
		} veth;

		veth.h_vlan_proto = skb->vlan_proto;
		veth.h_vlan_TCI = htons(skb_vlan_tag_get(skb));

		vlan_offset = offsetof(struct vlan_ethhdr, h_vlan_proto);

		ret = skb_copy_datagram_iter(skb, 0, iter, vlan_offset);
		if (ret || !iov_iter_count(iter))
			goto done;

		ret = copy_to_iter(&veth, sizeof(veth), iter);
		if (ret != sizeof(veth) || !iov_iter_count(iter))
			goto done;
	}

	skb_copy_datagram_iter(skb, vlan_offset, iter, skb->len - vlan_offset);

done:
	/* caller is in process context, */
	stats = get_cpu_ptr(tun->pcpu_stats);
	u64_stats_update_begin(&stats->syncp);
	stats->tx_packets++;
	stats->tx_bytes += skb->len + vlan_hlen;
	u64_stats_update_end(&stats->syncp);
	put_cpu_ptr(tun->pcpu_stats);

	return total;
}

static struct sk_buff *tun_ring_recv(struct tun_file *tfile, int noblock,
				     int *err)
{
	DECLARE_WAITQUEUE(wait, current);
	struct sk_buff *skb = NULL;
	int error = 0;

	skb = skb_array_consume(&tfile->tx_array);
	if (skb)
		goto out;
	if (noblock) {
		error = -EAGAIN;
		goto out;
	}

	add_wait_queue(&tfile->wq.wait, &wait);

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		skb = skb_array_consume(&tfile->tx_array);
		if (skb)
			break;
		if (signal_pending(current)) {
			error = -ERESTARTSYS;
			break;
		}
		if (tfile->socket.sk->sk_shutdown & RCV_SHUTDOWN) {
			error = -EFAULT;
			break;
		}

		schedule();
	}

	__set_current_state(TASK_RUNNING);
	remove_wait_queue(&tfile->wq.wait, &wait);

out:
	*err = error;
	return skb;
}

static ssize_t tun_do_read(struct tun_struct *tun, struct tun_file *tfile,
			   struct iov_iter *to,
			   int noblock, struct sk_buff *skb)
{
	ssize_t ret;
	int err;

	tun_debug(KERN_INFO, tun, "tun_do_read\n");

	if (!iov_iter_count(to)) {
		if (skb)
			kfree_skb(skb);
		return 0;
	}

	if (!skb) {
		/* Read frames from ring */
		skb = tun_ring_recv(tfile, noblock, &err);
		if (!skb)
			return err;
	}

	ret = tun_put_user(tun, tfile, skb, to);
	if (unlikely(ret < 0))
		kfree_skb(skb);
	else
		consume_skb(skb);

	return ret;
}

static ssize_t tun_chr_read_iter(struct kiocb *iocb, struct iov_iter *to)
{
	struct file *file = iocb->ki_filp;
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun = __tun_get(tfile);
	ssize_t len = iov_iter_count(to), ret;
	int noblock = 0;

	if (!tun)
		return -EBADFD;

	if ((file->f_flags & O_NONBLOCK) || (iocb->ki_flags & IOCB_NOWAIT))
		noblock = 1;

	ret = tun_do_read(tun, tfile, to, noblock, NULL);
	ret = min_t(ssize_t, ret, len);
	if (ret > 0)
		iocb->ki_pos = ret;
	tun_put(tun);
	return ret;
}

static void tun_free_netdev(struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);

	BUG_ON(!(list_empty(&tun->disabled)));
	free_percpu(tun->pcpu_stats);
	tun_flow_uninit(tun);
	security_tun_dev_free_security(tun->security);
}

static void tun_setup(struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);

	tun->owner = INVALID_UID;
	tun->group = INVALID_GID;

	dev->ethtool_ops = &tun_ethtool_ops;
	dev->needs_free_netdev = true;
	dev->priv_destructor = tun_free_netdev;
	/* We prefer our own queue length */
	dev->tx_queue_len = TUN_READQ_SIZE;
}

/* Trivial set of netlink ops to allow deleting tun or tap
 * device with netlink.
 */
static int tun_validate(struct nlattr *tb[], struct nlattr *data[],
			struct netlink_ext_ack *extack)
{
	NL_SET_ERR_MSG(extack,
		       "tun/tap creation via rtnetlink is not supported.");
	return -EOPNOTSUPP;
}

static struct rtnl_link_ops tun_link_ops __read_mostly = {
	.kind		= DRV_NAME,
	.priv_size	= sizeof(struct tun_struct),
	.setup		= tun_setup,
	.validate	= tun_validate,
};

static void tun_sock_write_space(struct sock *sk)
{
	struct tun_file *tfile;
	wait_queue_head_t *wqueue;

	if (!sock_writeable(sk))
		return;

	if (!test_and_clear_bit(SOCKWQ_ASYNC_NOSPACE, &sk->sk_socket->flags))
		return;

	wqueue = sk_sleep(sk);
	if (wqueue && waitqueue_active(wqueue))
		wake_up_interruptible_sync_poll(wqueue, POLLOUT |
						POLLWRNORM | POLLWRBAND);

	tfile = container_of(sk, struct tun_file, sk);
	kill_fasync(&tfile->fasync, SIGIO, POLL_OUT);
}

static int tun_sendmsg(struct socket *sock, struct msghdr *m, size_t total_len)
{
	int ret;
	struct tun_file *tfile = container_of(sock, struct tun_file, socket);
	struct tun_struct *tun = __tun_get(tfile);

	if (!tun)
		return -EBADFD;

	ret = tun_get_user(tun, tfile, m->msg_control, &m->msg_iter,
			   m->msg_flags & MSG_DONTWAIT,
			   m->msg_flags & MSG_MORE);
	tun_put(tun);
	return ret;
}

static int tun_recvmsg(struct socket *sock, struct msghdr *m, size_t total_len,
		       int flags)
{
	struct tun_file *tfile = container_of(sock, struct tun_file, socket);
	struct tun_struct *tun = __tun_get(tfile);
	struct sk_buff *skb = m->msg_control;
	int ret;

	if (!tun) {
		ret = -EBADFD;
		goto out_free_skb;
	}

	if (flags & ~(MSG_DONTWAIT|MSG_TRUNC|MSG_ERRQUEUE)) {
		ret = -EINVAL;
		goto out_put_tun;
	}
	if (flags & MSG_ERRQUEUE) {
		ret = sock_recv_errqueue(sock->sk, m, total_len,
					 SOL_PACKET, TUN_TX_TIMESTAMP);
		goto out;
	}
	ret = tun_do_read(tun, tfile, &m->msg_iter, flags & MSG_DONTWAIT, skb);
	if (ret > (ssize_t)total_len) {
		m->msg_flags |= MSG_TRUNC;
		ret = flags & MSG_TRUNC ? ret : total_len;
	}
out:
	tun_put(tun);
	return ret;

out_put_tun:
	tun_put(tun);
out_free_skb:
	if (skb)
		kfree_skb(skb);
	return ret;
}

static int tun_peek_len(struct socket *sock)
{
	struct tun_file *tfile = container_of(sock, struct tun_file, socket);
	struct tun_struct *tun;
	int ret = 0;

	tun = __tun_get(tfile);
	if (!tun)
		return 0;

	ret = skb_array_peek_len(&tfile->tx_array);
	tun_put(tun);

	return ret;
}

/* Ops structure to mimic raw sockets with tun */
static const struct proto_ops tun_socket_ops = {
	.peek_len = tun_peek_len,
	.sendmsg = tun_sendmsg,
	.recvmsg = tun_recvmsg,
};

static struct proto tun_proto = {
	.name		= "tun",
	.owner		= THIS_MODULE,
	.obj_size	= sizeof(struct tun_file),
};

static int tun_flags(struct tun_struct *tun)
{
	// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
	return tun->flags & (TUN_FEATURES | IFF_PERSIST | IFF_TUN | IFF_TAP | IFF_META_HDR);
	// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }
}

static ssize_t tun_show_flags(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return sprintf(buf, "0x%x\n", tun_flags(tun));
}

static ssize_t tun_show_owner(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return uid_valid(tun->owner)?
		sprintf(buf, "%u\n",
			from_kuid_munged(current_user_ns(), tun->owner)):
		sprintf(buf, "-1\n");
}

static ssize_t tun_show_group(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return gid_valid(tun->group) ?
		sprintf(buf, "%u\n",
			from_kgid_munged(current_user_ns(), tun->group)):
		sprintf(buf, "-1\n");
}

static DEVICE_ATTR(tun_flags, 0444, tun_show_flags, NULL);
static DEVICE_ATTR(owner, 0444, tun_show_owner, NULL);
static DEVICE_ATTR(group, 0444, tun_show_group, NULL);

static struct attribute *tun_dev_attrs[] = {
	&dev_attr_tun_flags.attr,
	&dev_attr_owner.attr,
	&dev_attr_group.attr,
	NULL
};

static const struct attribute_group tun_attr_group = {
	.attrs = tun_dev_attrs
};

static int tun_set_iff(struct net *net, struct file *file, struct ifreq *ifr)
{
	struct tun_struct *tun;
	struct tun_file *tfile = file->private_data;
	struct net_device *dev;
	int err;

	if (tfile->detached)
		return -EINVAL;

	dev = __dev_get_by_name(net, ifr->ifr_name);
	if (dev) {
		if (ifr->ifr_flags & IFF_TUN_EXCL)
			return -EBUSY;
		if ((ifr->ifr_flags & IFF_TUN) && dev->netdev_ops == &tun_netdev_ops)
			tun = netdev_priv(dev);
		else if ((ifr->ifr_flags & IFF_TAP) && dev->netdev_ops == &tap_netdev_ops)
			tun = netdev_priv(dev);
		else
			return -EINVAL;

		if (!!(ifr->ifr_flags & IFF_MULTI_QUEUE) !=
		    !!(tun->flags & IFF_MULTI_QUEUE))
			return -EINVAL;

		if (tun_not_capable(tun))
			return -EPERM;
		err = security_tun_dev_open(tun->security);
		if (err < 0)
			return err;

		err = tun_attach(tun, file, ifr->ifr_flags & IFF_NOFILTER, true);
		if (err < 0)
			return err;

		if (tun->flags & IFF_MULTI_QUEUE &&
		    (tun->numqueues + tun->numdisabled > 1)) {
			/* One or more queue has already been attached, no need
			 * to initialize the device again.
			 */
			return 0;
		}
	}
	else {
		char *name;
		unsigned long flags = 0;
		int queues = ifr->ifr_flags & IFF_MULTI_QUEUE ?
			     MAX_TAP_QUEUES : 1;

		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			return -EPERM;
		err = security_tun_dev_create();
		if (err < 0)
			return err;

		/* Set dev type */
		if (ifr->ifr_flags & IFF_TUN) {
			/* TUN device */
			flags |= IFF_TUN;
			name = "tun%d";
		} else if (ifr->ifr_flags & IFF_TAP) {
			/* TAP device */
			flags |= IFF_TAP;
			name = "tap%d";
		} else
			return -EINVAL;

		if (*ifr->ifr_name)
			name = ifr->ifr_name;

		dev = alloc_netdev_mqs(sizeof(struct tun_struct), name,
				       NET_NAME_UNKNOWN, tun_setup, queues,
				       queues);

		if (!dev)
			return -ENOMEM;
		err = dev_get_valid_name(net, dev, name);
		if (err < 0)
			goto err_free_dev;

		dev_net_set(dev, net);
		dev->rtnl_link_ops = &tun_link_ops;
		dev->ifindex = tfile->ifindex;
		dev->sysfs_groups[0] = &tun_attr_group;

		tun = netdev_priv(dev);
		tun->dev = dev;
		tun->flags = flags;
		tun->txflt.count = 0;
		tun->vnet_hdr_sz = sizeof(struct virtio_net_hdr);

		tun->align = NET_SKB_PAD;
		tun->filter_attached = false;
		tun->sndbuf = tfile->socket.sk->sk_sndbuf;
		tun->rx_batched = 0;

		tun->pcpu_stats = netdev_alloc_pcpu_stats(struct tun_pcpu_stats);
		if (!tun->pcpu_stats) {
			err = -ENOMEM;
			goto err_free_dev;
		}

		spin_lock_init(&tun->lock);

		err = security_tun_dev_alloc_security(&tun->security);
		if (err < 0)
			goto err_free_stat;

		tun_net_init(dev);
		tun_flow_init(tun);

		dev->hw_features = NETIF_F_SG | NETIF_F_FRAGLIST |
				   TUN_USER_FEATURES | NETIF_F_HW_VLAN_CTAG_TX |
				   NETIF_F_HW_VLAN_STAG_TX;
		dev->features = dev->hw_features | NETIF_F_LLTX;
		dev->vlan_features = dev->features &
				     ~(NETIF_F_HW_VLAN_CTAG_TX |
				       NETIF_F_HW_VLAN_STAG_TX);

		INIT_LIST_HEAD(&tun->disabled);
		err = tun_attach(tun, file, false, false);
		if (err < 0)
			goto err_free_flow;

		err = register_netdevice(tun->dev);
		if (err < 0)
			goto err_detach;
		/* free_netdev() won't check refcnt, to aovid race
		 * with dev_put() we need publish tun after registration.
		 */
		rcu_assign_pointer(tfile->tun, tun);
	}

	netif_carrier_on(tun->dev);

	tun_debug(KERN_INFO, tun, "tun_set_iff\n");
	// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
	if (ifr->ifr_flags & IFF_META_HDR) {
		tun->flags |= TUN_META_HDR;
	} else {
		tun->flags &= ~TUN_META_HDR;
	}
	// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }

	tun->flags = (tun->flags & ~TUN_FEATURES) |
		(ifr->ifr_flags & TUN_FEATURES);

	/* Make sure persistent devices do not get stuck in
	 * xoff state.
	 */
	if (netif_running(tun->dev))
		netif_tx_wake_all_queues(tun->dev);

	strcpy(ifr->ifr_name, tun->dev->name);
	return 0;

err_detach:
	tun_detach_all(dev);
	/* register_netdevice() already called tun_free_netdev() */
	goto err_free_dev;

err_free_flow:
	tun_flow_uninit(tun);
	security_tun_dev_free_security(tun->security);
err_free_stat:
	free_percpu(tun->pcpu_stats);
err_free_dev:
	free_netdev(dev);
	return err;
}

static void tun_get_iff(struct net *net, struct tun_struct *tun,
		       struct ifreq *ifr)
{
	tun_debug(KERN_INFO, tun, "tun_get_iff\n");

	strcpy(ifr->ifr_name, tun->dev->name);

	ifr->ifr_flags = tun_flags(tun);

}

/* This is like a cut-down ethtool ops, except done via tun fd so no
 * privs required. */
static int set_offload(struct tun_struct *tun, unsigned long arg)
{
	netdev_features_t features = 0;

	if (arg & TUN_F_CSUM) {
		features |= NETIF_F_HW_CSUM;
		arg &= ~TUN_F_CSUM;

		if (arg & (TUN_F_TSO4|TUN_F_TSO6)) {
			if (arg & TUN_F_TSO_ECN) {
				features |= NETIF_F_TSO_ECN;
				arg &= ~TUN_F_TSO_ECN;
			}
			if (arg & TUN_F_TSO4)
				features |= NETIF_F_TSO;
			if (arg & TUN_F_TSO6)
				features |= NETIF_F_TSO6;
			arg &= ~(TUN_F_TSO4|TUN_F_TSO6);
		}

		arg &= ~TUN_F_UFO;
	}

	/* This gives the user a way to test for new features in future by
	 * trying to set them. */
	if (arg)
		return -EINVAL;

	tun->set_features = features;
	tun->dev->wanted_features &= ~TUN_USER_FEATURES;
	tun->dev->wanted_features |= features;
	netdev_update_features(tun->dev);

	return 0;
}

static void tun_detach_filter(struct tun_struct *tun, int n)
{
	int i;
	struct tun_file *tfile;

	for (i = 0; i < n; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		lock_sock(tfile->socket.sk);
		sk_detach_filter(tfile->socket.sk);
		release_sock(tfile->socket.sk);
	}

	tun->filter_attached = false;
}

static int tun_attach_filter(struct tun_struct *tun)
{
	int i, ret = 0;
	struct tun_file *tfile;

	for (i = 0; i < tun->numqueues; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		lock_sock(tfile->socket.sk);
		ret = sk_attach_filter(&tun->fprog, tfile->socket.sk);
		release_sock(tfile->socket.sk);
		if (ret) {
			tun_detach_filter(tun, i);
			return ret;
		}
	}

	tun->filter_attached = true;
	return ret;
}

static void tun_set_sndbuf(struct tun_struct *tun)
{
	struct tun_file *tfile;
	int i;

	for (i = 0; i < tun->numqueues; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		tfile->socket.sk->sk_sndbuf = tun->sndbuf;
	}
}

static int tun_set_queue(struct file *file, struct ifreq *ifr)
{
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun;
	int ret = 0;

	rtnl_lock();

	if (ifr->ifr_flags & IFF_ATTACH_QUEUE) {
		tun = tfile->detached;
		if (!tun) {
			ret = -EINVAL;
			goto unlock;
		}
		ret = security_tun_dev_attach_queue(tun->security);
		if (ret < 0)
			goto unlock;
		ret = tun_attach(tun, file, false, true);
	} else if (ifr->ifr_flags & IFF_DETACH_QUEUE) {
		tun = rtnl_dereference(tfile->tun);
		if (!tun || !(tun->flags & IFF_MULTI_QUEUE) || tfile->detached)
			ret = -EINVAL;
		else
			__tun_detach(tfile, false);
	} else
		ret = -EINVAL;

unlock:
	rtnl_unlock();
	return ret;
}

/* Return correct value for tun->dev->addr_len based on tun->dev->type. */
static unsigned char tun_get_addr_len(unsigned short type)
{
	switch (type) {
	case ARPHRD_IP6GRE:
	case ARPHRD_TUNNEL6:
		return sizeof(struct in6_addr);
	case ARPHRD_IPGRE:
	case ARPHRD_TUNNEL:
	case ARPHRD_SIT:
		return 4;
	case ARPHRD_ETHER:
		return ETH_ALEN;
	case ARPHRD_IEEE802154:
	case ARPHRD_IEEE802154_MONITOR:
		return IEEE802154_EXTENDED_ADDR_LEN;
	case ARPHRD_PHONET_PIPE:
	case ARPHRD_PPP:
	case ARPHRD_NONE:
		return 0;
	case ARPHRD_6LOWPAN:
		return EUI64_ADDR_LEN;
	case ARPHRD_FDDI:
		return FDDI_K_ALEN;
	case ARPHRD_HIPPI:
		return HIPPI_ALEN;
	case ARPHRD_IEEE802:
		return FC_ALEN;
	case ARPHRD_ROSE:
		return ROSE_ADDR_LEN;
	case ARPHRD_NETROM:
		return AX25_ADDR_LEN;
	case ARPHRD_LOCALTLK:
		return LTALK_ALEN;
	default:
		return 0;
	}
}

static long __tun_chr_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg, int ifreq_len)
{
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun;
	void __user* argp = (void __user*)arg;
	struct ifreq ifr;
	kuid_t owner;
	kgid_t group;
	int sndbuf;
	int vnet_hdr_sz;
	unsigned int ifindex;
	int le;
	int ret;
	// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
	int knox_flag = 0;
	int tun_meta_param;
	int tun_meta_value;
	// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }

	if (cmd == TUNSETIFF || cmd == TUNSETQUEUE || _IOC_TYPE(cmd) == SOCK_IOC_TYPE) {
		if (copy_from_user(&ifr, argp, ifreq_len))
			return -EFAULT;
	} else {
		memset(&ifr, 0, sizeof(ifr));
	}
	if (cmd == TUNGETFEATURES) {
		/* Currently this just means: "what IFF flags are valid?".
		 * This is needed because we never checked for invalid flags on
		 * TUNSETIFF.
		 */
		// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
		knox_flag |= IFF_META_HDR;
		return put_user(IFF_TUN | IFF_TAP | TUN_FEATURES | knox_flag,
				(unsigned int __user*)argp);
		// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }
	} else if (cmd == TUNSETQUEUE)
		return tun_set_queue(file, &ifr);

	ret = 0;
	rtnl_lock();

	tun = __tun_get(tfile);
	if (cmd == TUNSETIFF) {
		ret = -EEXIST;
		if (tun)
			goto unlock;

		ifr.ifr_name[IFNAMSIZ-1] = '\0';

		ret = tun_set_iff(sock_net(&tfile->sk), file, &ifr);

		if (ret)
			goto unlock;

		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		goto unlock;
	}
	if (cmd == TUNSETIFINDEX) {
		ret = -EPERM;
		if (tun)
			goto unlock;

		ret = -EFAULT;
		if (copy_from_user(&ifindex, argp, sizeof(ifindex)))
			goto unlock;

		ret = 0;
		tfile->ifindex = ifindex;
		goto unlock;
	}

	ret = -EBADFD;
	if (!tun)
		goto unlock;

	tun_debug(KERN_INFO, tun, "tun_chr_ioctl cmd %u\n", cmd);

	ret = 0;
	switch (cmd) {
	case TUNGETIFF:
		tun_get_iff(current->nsproxy->net_ns, tun, &ifr);

		if (tfile->detached)
			ifr.ifr_flags |= IFF_DETACH_QUEUE;
		if (!tfile->socket.sk->sk_filter)
			ifr.ifr_flags |= IFF_NOFILTER;

		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		break;

	case TUNSETNOCSUM:
		/* Disable/Enable checksum */

		/* [unimplemented] */
		tun_debug(KERN_INFO, tun, "ignored: set checksum %s\n",
			  arg ? "disabled" : "enabled");
		break;

	case TUNSETPERSIST:
		/* Disable/Enable persist mode. Keep an extra reference to the
		 * module to prevent the module being unprobed.
		 */
		if (arg && !(tun->flags & IFF_PERSIST)) {
			tun->flags |= IFF_PERSIST;
			__module_get(THIS_MODULE);
		}
		if (!arg && (tun->flags & IFF_PERSIST)) {
			tun->flags &= ~IFF_PERSIST;
			module_put(THIS_MODULE);
		}

		tun_debug(KERN_INFO, tun, "persist %s\n",
			  arg ? "enabled" : "disabled");
		break;

	case TUNSETOWNER:
		/* Set owner of the device */
		owner = make_kuid(current_user_ns(), arg);
		if (!uid_valid(owner)) {
			ret = -EINVAL;
			break;
		}
		tun->owner = owner;
		tun_debug(KERN_INFO, tun, "owner set to %u\n",
			  from_kuid(&init_user_ns, tun->owner));
		break;

	case TUNSETGROUP:
		/* Set group of the device */
		group = make_kgid(current_user_ns(), arg);
		if (!gid_valid(group)) {
			ret = -EINVAL;
			break;
		}
		tun->group = group;
		tun_debug(KERN_INFO, tun, "group set to %u\n",
			  from_kgid(&init_user_ns, tun->group));
		break;

	case TUNSETLINK:
		/* Only allow setting the type when the interface is down */
		if (tun->dev->flags & IFF_UP) {
			tun_debug(KERN_INFO, tun,
				  "Linktype set failed because interface is up\n");
			ret = -EBUSY;
		} else {
			tun->dev->type = (int) arg;
			tun->dev->addr_len = tun_get_addr_len(tun->dev->type);
			tun_debug(KERN_INFO, tun, "linktype set to %d\n",
				  tun->dev->type);
			ret = 0;
		}
		break;

#ifdef TUN_DEBUG
	case TUNSETDEBUG:
		tun->debug = arg;
		break;
#endif
	case TUNSETOFFLOAD:
		ret = set_offload(tun, arg);
		break;

	case TUNSETTXFILTER:
		/* Can be set only for TAPs */
		ret = -EINVAL;
		if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
			break;
		ret = update_filter(&tun->txflt, (void __user *)arg);
		break;

	case SIOCGIFHWADDR:
		/* Get hw address */
		memcpy(ifr.ifr_hwaddr.sa_data, tun->dev->dev_addr, ETH_ALEN);
		ifr.ifr_hwaddr.sa_family = tun->dev->type;
		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		break;

	case SIOCSIFHWADDR:
		/* Set hw address */
		tun_debug(KERN_DEBUG, tun, "set hw address: %pM\n",
			  ifr.ifr_hwaddr.sa_data);

		ret = dev_set_mac_address(tun->dev, &ifr.ifr_hwaddr);
		break;

	case TUNGETSNDBUF:
		sndbuf = tfile->socket.sk->sk_sndbuf;
		if (copy_to_user(argp, &sndbuf, sizeof(sndbuf)))
			ret = -EFAULT;
		break;

	case TUNSETSNDBUF:
		if (copy_from_user(&sndbuf, argp, sizeof(sndbuf))) {
			ret = -EFAULT;
			break;
		}
		if (sndbuf <= 0) {
			ret = -EINVAL;
			break;
		}

		tun->sndbuf = sndbuf;
		tun_set_sndbuf(tun);
		break;

	case TUNGETVNETHDRSZ:
		vnet_hdr_sz = tun->vnet_hdr_sz;
		if (copy_to_user(argp, &vnet_hdr_sz, sizeof(vnet_hdr_sz)))
			ret = -EFAULT;
		break;

	// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN {
	case TUNGETMETAPARAM:

		if (copy_from_user(&tun_meta_param, argp,
				   sizeof(tun_meta_param))) {
			ret = -EFAULT;
			break;
		}

		ret = 0;
		switch (tun_meta_param) {
		case TUN_GET_META_HDR_SZ:
			tun_meta_value = TUN_META_HDR_SZ;
			break;

		case TUN_GET_META_MARK_OFFSET:
			tun_meta_value = TUN_META_MARK_OFFSET;
			break;

		default:
			ret = -EINVAL;
			break;
		}

		if (!ret) {
			if (copy_to_user(argp, &tun_meta_value,
					 sizeof(tun_meta_value)))
				ret = -EFAULT;
		}
		break;
	// SEC_PRODUCT_FEATURE_KNOX_SUPPORT_VPN }
	case TUNSETVNETHDRSZ:
		if (copy_from_user(&vnet_hdr_sz, argp, sizeof(vnet_hdr_sz))) {
			ret = -EFAULT;
			break;
		}
		if (vnet_hdr_sz < (int)sizeof(struct virtio_net_hdr)) {
			ret = -EINVAL;
			break;
		}

		tun->vnet_hdr_sz = vnet_hdr_sz;
		break;

	case TUNGETVNETLE:
		le = !!(tun->flags & TUN_VNET_LE);
		if (put_user(le, (int __user *)argp))
			ret = -EFAULT;
		break;

	case TUNSETVNETLE:
		if (get_user(le, (int __user *)argp)) {
			ret = -EFAULT;
			break;
		}
		if (le)
			tun->flags |= TUN_VNET_LE;
		else
			tun->flags &= ~TUN_VNET_LE;
		break;

	case TUNGETVNETBE:
		ret = tun_get_vnet_be(tun, argp);
		break;

	case TUNSETVNETBE:
		ret = tun_set_vnet_be(tun, argp);
		break;

	case TUNATTACHFILTER:
		/* Can be set only for TAPs */
		ret = -EINVAL;
		if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
			break;
		ret = -EFAULT;
		if (copy_from_user(&tun->fprog, argp, sizeof(tun->fprog)))
			break;

		ret = tun_attach_filter(tun);
		break;

	case TUNDETACHFILTER:
		/* Can be set only for TAPs */
		ret = -EINVAL;
		if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
			break;
		ret = 0;
		tun_detach_filter(tun, tun->numqueues);
		break;

	case TUNGETFILTER:
		ret = -EINVAL;
		if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
			break;
		ret = -EFAULT;
		if (copy_to_user(argp, &tun->fprog, sizeof(tun->fprog)))
			break;
		ret = 0;
		break;

	default:
		ret = -EINVAL;
		break;
	}

unlock:
	rtnl_unlock();
	if (tun)
		tun_put(tun);
	return ret;
}

static long tun_chr_ioctl(struct file *file,
			  unsigned int cmd, unsigned long arg)
{
	return __tun_chr_ioctl(file, cmd, arg, sizeof (struct ifreq));
}

#ifdef CONFIG_COMPAT
static long tun_chr_compat_ioctl(struct file *file,
			 unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case TUNSETIFF:
	case TUNGETIFF:
	case TUNSETTXFILTER:
	case TUNGETSNDBUF:
	case TUNSETSNDBUF:
	case SIOCGIFHWADDR:
	case SIOCSIFHWADDR:
		arg = (unsigned long)compat_ptr(arg);
		break;
	default:
		arg = (compat_ulong_t)arg;
		break;
	}

	/*
	 * compat_ifreq is shorter than ifreq, so we must not access beyond
	 * the end of that structure. All fields that are used in this
	 * driver are compatible though, we don't need to convert the
	 * contents.
	 */
	return __tun_chr_ioctl(file, cmd, arg, sizeof(struct compat_ifreq));
}
#endif /* CONFIG_COMPAT */

static int tun_chr_fasync(int fd, struct file *file, int on)
{
	struct tun_file *tfile = file->private_data;
	int ret;

	if ((ret = fasync_helper(fd, file, on, &tfile->fasync)) < 0)
		goto out;

	if (on) {
		__f_setown(file, task_pid(current), PIDTYPE_PID, 0);
		tfile->flags |= TUN_FASYNC;
	} else
		tfile->flags &= ~TUN_FASYNC;
	ret = 0;
out:
	return ret;
}

static int tun_chr_open(struct inode *inode, struct file * file)
{
	struct net *net = current->nsproxy->net_ns;
	struct tun_file *tfile;

	DBG1(KERN_INFO, "tunX: tun_chr_open\n");

	tfile = (struct tun_file *)sk_alloc(net, AF_UNSPEC, GFP_KERNEL,
					    &tun_proto, 0);
	if (!tfile)
		return -ENOMEM;
	if (skb_array_init(&tfile->tx_array, 0, GFP_KERNEL)) {
		sk_free(&tfile->sk);
		return -ENOMEM;
	}

	RCU_INIT_POINTER(tfile->tun, NULL);
	tfile->flags = 0;
	tfile->ifindex = 0;

	init_waitqueue_head(&tfile->wq.wait);
	RCU_INIT_POINTER(tfile->socket.wq, &tfile->wq);

	tfile->socket.file = file;
	tfile->socket.ops = &tun_socket_ops;

	sock_init_data(&tfile->socket, &tfile->sk);

	tfile->sk.sk_write_space = tun_sock_write_space;
	tfile->sk.sk_sndbuf = INT_MAX;

	file->private_data = tfile;
	INIT_LIST_HEAD(&tfile->next);

	sock_set_flag(&tfile->sk, SOCK_ZEROCOPY);

	return 0;
}

static int tun_chr_close(struct inode *inode, struct file *file)
{
	struct tun_file *tfile = file->private_data;

	tun_detach(tfile, true);

	return 0;
}

#ifdef CONFIG_PROC_FS
static void tun_chr_show_fdinfo(struct seq_file *m, struct file *f)
{
	struct tun_struct *tun;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));

	rtnl_lock();
	tun = tun_get(f);
	if (tun)
		tun_get_iff(current->nsproxy->net_ns, tun, &ifr);
	rtnl_unlock();

	if (tun)
		tun_put(tun);

	seq_printf(m, "iff:\t%s\n", ifr.ifr_name);
}
#endif

static const struct file_operations tun_fops = {
	.owner	= THIS_MODULE,
	.llseek = no_llseek,
	.read_iter  = tun_chr_read_iter,
	.write_iter = tun_chr_write_iter,
	.poll	= tun_chr_poll,
	.unlocked_ioctl	= tun_chr_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = tun_chr_compat_ioctl,
#endif
	.open	= tun_chr_open,
	.release = tun_chr_close,
	.fasync = tun_chr_fasync,
#ifdef CONFIG_PROC_FS
	.show_fdinfo = tun_chr_show_fdinfo,
#endif
};

static struct miscdevice tun_miscdev = {
	.minor = TUN_MINOR,
	.name = "tun",
	.nodename = "net/tun",
	.fops = &tun_fops,
};

/* ethtool interface */

static int tun_get_link_ksettings(struct net_device *dev,
				  struct ethtool_link_ksettings *cmd)
{
	ethtool_link_ksettings_zero_link_mode(cmd, supported);
	ethtool_link_ksettings_zero_link_mode(cmd, advertising);
	cmd->base.speed		= SPEED_10;
	cmd->base.duplex	= DUPLEX_FULL;
	cmd->base.port		= PORT_TP;
	cmd->base.phy_address	= 0;
	cmd->base.autoneg	= AUTONEG_DISABLE;
	return 0;
}

static void tun_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct tun_struct *tun = netdev_priv(dev);

	strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
	strlcpy(info->version, DRV_VERSION, sizeof(info->version));

	switch (tun->flags & TUN_TYPE_MASK) {
	case IFF_TUN:
		strlcpy(info->bus_info, "tun", sizeof(info->bus_info));
		break;
	case IFF_TAP:
		strlcpy(info->bus_info, "tap", sizeof(info->bus_info));
		break;
	}
}

static u32 tun_get_msglevel(struct net_device *dev)
{
#ifdef TUN_DEBUG
	struct tun_struct *tun = netdev_priv(dev);
	return tun->debug;
#else
	return -EOPNOTSUPP;
#endif
}

static void tun_set_msglevel(struct net_device *dev, u32 value)
{
#ifdef TUN_DEBUG
	struct tun_struct *tun = netdev_priv(dev);
	tun->debug = value;
#endif
}

static int tun_get_coalesce(struct net_device *dev,
			    struct ethtool_coalesce *ec)
{
	struct tun_struct *tun = netdev_priv(dev);

	ec->rx_max_coalesced_frames = tun->rx_batched;

	return 0;
}

static int tun_set_coalesce(struct net_device *dev,
			    struct ethtool_coalesce *ec)
{
	struct tun_struct *tun = netdev_priv(dev);

	if (ec->rx_max_coalesced_frames > NAPI_POLL_WEIGHT)
		tun->rx_batched = NAPI_POLL_WEIGHT;
	else
		tun->rx_batched = ec->rx_max_coalesced_frames;

	return 0;
}

static const struct ethtool_ops tun_ethtool_ops = {
	.get_drvinfo	= tun_get_drvinfo,
	.get_msglevel	= tun_get_msglevel,
	.set_msglevel	= tun_set_msglevel,
	.get_link	= ethtool_op_get_link,
	.get_ts_info	= ethtool_op_get_ts_info,
	.get_coalesce   = tun_get_coalesce,
	.set_coalesce   = tun_set_coalesce,
	.get_link_ksettings = tun_get_link_ksettings,
};

static int tun_queue_resize(struct tun_struct *tun)
{
	struct net_device *dev = tun->dev;
	struct tun_file *tfile;
	struct skb_array **arrays;
	int n = tun->numqueues + tun->numdisabled;
	int ret, i;

	arrays = kmalloc_array(n, sizeof(*arrays), GFP_KERNEL);
	if (!arrays)
		return -ENOMEM;

	for (i = 0; i < tun->numqueues; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		arrays[i] = &tfile->tx_array;
	}
	list_for_each_entry(tfile, &tun->disabled, next)
		arrays[i++] = &tfile->tx_array;

	ret = skb_array_resize_multiple(arrays, n,
					dev->tx_queue_len, GFP_KERNEL);

	kfree(arrays);
	return ret;
}

static int tun_device_event(struct notifier_block *unused,
			    unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct tun_struct *tun = netdev_priv(dev);
	int i;

	if (dev->rtnl_link_ops != &tun_link_ops)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_CHANGE_TX_QUEUE_LEN:
		if (tun_queue_resize(tun))
			return NOTIFY_BAD;
		break;
	case NETDEV_UP:
		for (i = 0; i < tun->numqueues; i++) {
			struct tun_file *tfile;

			tfile = rtnl_dereference(tun->tfiles[i]);
			tfile->socket.sk->sk_write_space(tfile->socket.sk);
		}
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block tun_notifier_block __read_mostly = {
	.notifier_call	= tun_device_event,
};

static int __init tun_init(void)
{
	int ret = 0;

	pr_info("%s, %s\n", DRV_DESCRIPTION, DRV_VERSION);

	ret = rtnl_link_register(&tun_link_ops);
	if (ret) {
		pr_err("Can't register link_ops\n");
		goto err_linkops;
	}

	ret = misc_register(&tun_miscdev);
	if (ret) {
		pr_err("Can't register misc device %d\n", TUN_MINOR);
		goto err_misc;
	}

	ret = register_netdevice_notifier(&tun_notifier_block);
	if (ret) {
		pr_err("Can't register netdevice notifier\n");
		goto err_notifier;
	}

	return  0;

err_notifier:
	misc_deregister(&tun_miscdev);
err_misc:
	rtnl_link_unregister(&tun_link_ops);
err_linkops:
	return ret;
}

static void tun_cleanup(void)
{
	misc_deregister(&tun_miscdev);
	rtnl_link_unregister(&tun_link_ops);
	unregister_netdevice_notifier(&tun_notifier_block);
}

/* Get an underlying socket object from tun file.  Returns error unless file is
 * attached to a device.  The returned object works like a packet socket, it
 * can be used for sock_sendmsg/sock_recvmsg.  The caller is responsible for
 * holding a reference to the file for as long as the socket is in use. */
struct socket *tun_get_socket(struct file *file)
{
	struct tun_file *tfile;
	if (file->f_op != &tun_fops)
		return ERR_PTR(-EINVAL);
	tfile = file->private_data;
	if (!tfile)
		return ERR_PTR(-EBADFD);
	return &tfile->socket;
}
EXPORT_SYMBOL_GPL(tun_get_socket);

struct skb_array *tun_get_skb_array(struct file *file)
{
	struct tun_file *tfile;

	if (file->f_op != &tun_fops)
		return ERR_PTR(-EINVAL);
	tfile = file->private_data;
	if (!tfile)
		return ERR_PTR(-EBADFD);
	return &tfile->tx_array;
}
EXPORT_SYMBOL_GPL(tun_get_skb_array);

module_init(tun_init);
module_exit(tun_cleanup);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR(DRV_COPYRIGHT);
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(TUN_MINOR);
MODULE_ALIAS("devname:net/tun");
