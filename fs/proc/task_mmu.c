// SPDX-License-Identifier: GPL-2.0
#include <linux/mm.h>
#include <linux/vmacache.h>
#include <linux/hugetlb.h>
#include <linux/huge_mm.h>
#include <linux/mount.h>
#include <linux/seq_file.h>
#include <linux/highmem.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/pagemap.h>
#include <linux/mempolicy.h>
#include <linux/rmap.h>
#include <linux/swap.h>
#include <linux/sched/mm.h>
#include <linux/swapops.h>
#include <linux/mmu_notifier.h>
#include <linux/page_idle.h>
#include <linux/shmem_fs.h>
#include <linux/uaccess.h>
#include <linux/mm_inline.h>
#include <linux/freezer.h>

#include <asm/elf.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>
#include "internal.h"

#ifdef CONFIG_ZRAM_LRU_WRITEBACK
#include <linux/delay.h>
#include "../../drivers/block/zram/zram_drv.h"
#endif

void task_mem(struct seq_file *m, struct mm_struct *mm)
{
	unsigned long text, lib, swap, ptes, pmds, anon, file, shmem;
	unsigned long hiwater_vm, total_vm, hiwater_rss, total_rss;

	anon = get_mm_counter(mm, MM_ANONPAGES);
	file = get_mm_counter(mm, MM_FILEPAGES);
	shmem = get_mm_counter(mm, MM_SHMEMPAGES);

	/*
	 * Note: to minimize their overhead, mm maintains hiwater_vm and
	 * hiwater_rss only when about to *lower* total_vm or rss.  Any
	 * collector of these hiwater stats must therefore get total_vm
	 * and rss too, which will usually be the higher.  Barriers? not
	 * worth the effort, such snapshots can always be inconsistent.
	 */
	hiwater_vm = total_vm = mm->total_vm;
	if (hiwater_vm < mm->hiwater_vm)
		hiwater_vm = mm->hiwater_vm;
	hiwater_rss = total_rss = anon + file + shmem;
	if (hiwater_rss < mm->hiwater_rss)
		hiwater_rss = mm->hiwater_rss;

	text = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK)) >> 10;
	lib = (mm->exec_vm << (PAGE_SHIFT-10)) - text;
	swap = get_mm_counter(mm, MM_SWAPENTS);
	ptes = PTRS_PER_PTE * sizeof(pte_t) * atomic_long_read(&mm->nr_ptes);
	pmds = PTRS_PER_PMD * sizeof(pmd_t) * mm_nr_pmds(mm);
	seq_printf(m,
		"VmPeak:\t%8lu kB\n"
		"VmSize:\t%8lu kB\n"
		"VmLck:\t%8lu kB\n"
		"VmPin:\t%8lu kB\n"
		"VmHWM:\t%8lu kB\n"
		"VmRSS:\t%8lu kB\n"
		"RssAnon:\t%8lu kB\n"
		"RssFile:\t%8lu kB\n"
		"RssShmem:\t%8lu kB\n"
		"VmData:\t%8lu kB\n"
		"VmStk:\t%8lu kB\n"
		"VmExe:\t%8lu kB\n"
		"VmLib:\t%8lu kB\n"
		"VmPTE:\t%8lu kB\n"
		"VmPMD:\t%8lu kB\n"
		"VmSwap:\t%8lu kB\n",
		hiwater_vm << (PAGE_SHIFT-10),
		total_vm << (PAGE_SHIFT-10),
		mm->locked_vm << (PAGE_SHIFT-10),
		mm->pinned_vm << (PAGE_SHIFT-10),
		hiwater_rss << (PAGE_SHIFT-10),
		total_rss << (PAGE_SHIFT-10),
		anon << (PAGE_SHIFT-10),
		file << (PAGE_SHIFT-10),
		shmem << (PAGE_SHIFT-10),
		mm->data_vm << (PAGE_SHIFT-10),
		mm->stack_vm << (PAGE_SHIFT-10), text, lib,
		ptes >> 10,
		pmds >> 10,
		swap << (PAGE_SHIFT-10));
	hugetlb_report_usage(m, mm);
}

unsigned long task_vsize(struct mm_struct *mm)
{
	return PAGE_SIZE * mm->total_vm;
}

unsigned long task_statm(struct mm_struct *mm,
			 unsigned long *shared, unsigned long *text,
			 unsigned long *data, unsigned long *resident)
{
	*shared = get_mm_counter(mm, MM_FILEPAGES) +
			get_mm_counter(mm, MM_SHMEMPAGES);
	*text = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK))
								>> PAGE_SHIFT;
	*data = mm->data_vm + mm->stack_vm;
	*resident = *shared + get_mm_counter(mm, MM_ANONPAGES);
	return mm->total_vm;
}
void task_statlmkd(struct mm_struct *mm, unsigned long *size,
			 unsigned long *resident, unsigned long *swapresident)
{

	int zswap_stored_pages_temp=0;

	*size = mm->total_vm;
	*resident = get_mm_counter(mm, MM_FILEPAGES) +
			get_mm_counter(mm, MM_SHMEMPAGES) +
			get_mm_counter(mm, MM_ANONPAGES);

	*swapresident = get_mm_counter(mm, MM_SWAPENTS);
}
#ifdef CONFIG_NUMA
/*
 * Save get_task_policy() for show_numa_map().
 */
static void hold_task_mempolicy(struct proc_maps_private *priv)
{
	struct task_struct *task = priv->task;

	task_lock(task);
	priv->task_mempolicy = get_task_policy(task);
	mpol_get(priv->task_mempolicy);
	task_unlock(task);
}
static void release_task_mempolicy(struct proc_maps_private *priv)
{
	mpol_put(priv->task_mempolicy);
}
#else
static void hold_task_mempolicy(struct proc_maps_private *priv)
{
}
static void release_task_mempolicy(struct proc_maps_private *priv)
{
}
#endif

static void seq_print_vma_name(struct seq_file *m, struct vm_area_struct *vma)
{
	const char __user *name = vma_get_anon_name(vma);
	struct mm_struct *mm = vma->vm_mm;

	unsigned long page_start_vaddr;
	unsigned long page_offset;
	unsigned long num_pages;
	unsigned long max_len = NAME_MAX;
	int i;

	page_start_vaddr = (unsigned long)name & PAGE_MASK;
	page_offset = (unsigned long)name - page_start_vaddr;
	num_pages = DIV_ROUND_UP(page_offset + max_len, PAGE_SIZE);

	seq_puts(m, "[anon:");

	for (i = 0; i < num_pages; i++) {
		int len;
		int write_len;
		const char *kaddr;
		long pages_pinned;
		struct page *page;

		pages_pinned = get_user_pages_remote(current, mm,
				page_start_vaddr, 1, 0, &page, NULL, NULL);
		if (pages_pinned < 1) {
			seq_puts(m, "<fault>]");
			return;
		}

		kaddr = (const char *)kmap(page);
		len = min(max_len, PAGE_SIZE - page_offset);
		write_len = strnlen(kaddr + page_offset, len);
		seq_write(m, kaddr + page_offset, write_len);
		kunmap(page);
		put_page(page);

		/* if strnlen hit a null terminator then we're done */
		if (write_len != len)
			break;

		max_len -= len;
		page_offset = 0;
		page_start_vaddr += PAGE_SIZE;
	}

	seq_putc(m, ']');
}

static void vma_stop(struct proc_maps_private *priv)
{
	struct mm_struct *mm = priv->mm;

	release_task_mempolicy(priv);
	up_read(&mm->mmap_sem);
	mmput(mm);
}

static struct vm_area_struct *
m_next_vma(struct proc_maps_private *priv, struct vm_area_struct *vma)
{
	if (vma == priv->tail_vma)
		return NULL;
	return vma->vm_next ?: priv->tail_vma;
}

static void m_cache_vma(struct seq_file *m, struct vm_area_struct *vma)
{
	if (m->count < m->size)	/* vma is copied successfully */
		m->version = m_next_vma(m->private, vma) ? vma->vm_end : -1UL;
}

static void *m_start(struct seq_file *m, loff_t *ppos)
{
	struct proc_maps_private *priv = m->private;
	unsigned long last_addr = m->version;
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	unsigned int pos = *ppos;

	/* See m_cache_vma(). Zero at the start or after lseek. */
	if (last_addr == -1UL)
		return NULL;

	priv->task = get_proc_task(priv->inode);
	if (!priv->task)
		return ERR_PTR(-ESRCH);

	mm = priv->mm;
	if (!mm || !mmget_not_zero(mm))
		return NULL;

	down_read(&mm->mmap_sem);
	hold_task_mempolicy(priv);
	priv->tail_vma = get_gate_vma(mm);

	if (last_addr) {
		vma = find_vma(mm, last_addr - 1);
		if (vma && vma->vm_start <= last_addr)
			vma = m_next_vma(priv, vma);
		if (vma)
			return vma;
	}

	m->version = 0;
	if (pos < mm->map_count) {
		for (vma = mm->mmap; pos; pos--) {
			m->version = vma->vm_start;
			vma = vma->vm_next;
		}
		return vma;
	}

	/* we do not bother to update m->version in this case */
	if (pos == mm->map_count && priv->tail_vma)
		return priv->tail_vma;

	vma_stop(priv);
	return NULL;
}

static void *m_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct proc_maps_private *priv = m->private;
	struct vm_area_struct *next;

	(*pos)++;
	next = m_next_vma(priv, v);
	if (!next)
		vma_stop(priv);
	return next;
}

static void m_stop(struct seq_file *m, void *v)
{
	struct proc_maps_private *priv = m->private;

	if (!IS_ERR_OR_NULL(v))
		vma_stop(priv);
	if (priv->task) {
		put_task_struct(priv->task);
		priv->task = NULL;
	}
}

static int proc_maps_open(struct inode *inode, struct file *file,
			const struct seq_operations *ops, int psize)
{
	struct proc_maps_private *priv = __seq_open_private(file, ops, psize);

	if (!priv)
		return -ENOMEM;

	priv->inode = inode;
	priv->mm = proc_mem_open(inode, PTRACE_MODE_READ);
	if (IS_ERR(priv->mm)) {
		int err = PTR_ERR(priv->mm);

		seq_release_private(inode, file);
		return err;
	}

	return 0;
}

static int proc_map_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = file->private_data;
	struct proc_maps_private *priv = seq->private;

	if (priv->mm)
		mmdrop(priv->mm);

	return seq_release_private(inode, file);
}

static int do_maps_open(struct inode *inode, struct file *file,
			const struct seq_operations *ops)
{
	return proc_maps_open(inode, file, ops,
				sizeof(struct proc_maps_private));
}

/*
 * Indicate if the VMA is a stack for the given task; for
 * /proc/PID/maps that is the stack of the main task.
 */
static int is_stack(struct vm_area_struct *vma)
{
	/*
	 * We make no effort to guess what a given thread considers to be
	 * its "stack".  It's not even well-defined for programs written
	 * languages like Go.
	 */
	return vma->vm_start <= vma->vm_mm->start_stack &&
		vma->vm_end >= vma->vm_mm->start_stack;
}

static void show_vma_header_prefix(struct seq_file *m,
				   unsigned long start, unsigned long end,
				   vm_flags_t flags, unsigned long long pgoff,
				   dev_t dev, unsigned long ino)
{
	seq_setwidth(m, 25 + sizeof(void *) * 6 - 1);
	seq_printf(m, "%08lx-%08lx %c%c%c%c %08llx %02x:%02x %lu ",
		   start,
		   end,
		   flags & VM_READ ? 'r' : '-',
		   flags & VM_WRITE ? 'w' : '-',
		   flags & VM_EXEC ? 'x' : '-',
		   flags & VM_MAYSHARE ? 's' : 'p',
		   pgoff,
		   MAJOR(dev), MINOR(dev), ino);
}

static void
show_map_vma(struct seq_file *m, struct vm_area_struct *vma)
{
	struct mm_struct *mm = vma->vm_mm;
	struct file *file = vma->vm_file;
	vm_flags_t flags = vma->vm_flags;
	unsigned long ino = 0;
	unsigned long long pgoff = 0;
	unsigned long start, end;
	dev_t dev = 0;
	const char *name = NULL;

	if (file) {
		struct inode *inode = file_inode(vma->vm_file);
		dev = inode->i_sb->s_dev;
		ino = inode->i_ino;
		pgoff = ((loff_t)vma->vm_pgoff) << PAGE_SHIFT;
	}

	start = vma->vm_start;
	end = vma->vm_end;
	show_vma_header_prefix(m, start, end, flags, pgoff, dev, ino);

	/*
	 * Print the dentry name for named mappings, and a
	 * special [heap] marker for the heap:
	 */
	if (file) {
		seq_pad(m, ' ');
		seq_file_path(m, file, "\n");
		goto done;
	}

	if (vma->vm_ops && vma->vm_ops->name) {
		name = vma->vm_ops->name(vma);
		if (name)
			goto done;
	}

	name = arch_vma_name(vma);
	if (!name) {
		if (!mm) {
			name = "[vdso]";
			goto done;
		}

		if (vma->vm_start <= mm->brk &&
		    vma->vm_end >= mm->start_brk) {
			name = "[heap]";
			goto done;
		}

		if (is_stack(vma)) {
			name = "[stack]";
			goto done;
		}

		if (vma_get_anon_name(vma)) {
			seq_pad(m, ' ');
			seq_print_vma_name(m, vma);
		}
	}

done:
	if (name) {
		seq_pad(m, ' ');
		seq_puts(m, name);
	}
	seq_putc(m, '\n');
}

static int show_map(struct seq_file *m, void *v)
{
	show_map_vma(m, v);
	m_cache_vma(m, v);
	return 0;
}

static const struct seq_operations proc_pid_maps_op = {
	.start	= m_start,
	.next	= m_next,
	.stop	= m_stop,
	.show	= show_map
};

static int pid_maps_open(struct inode *inode, struct file *file)
{
	return do_maps_open(inode, file, &proc_pid_maps_op);
}

const struct file_operations proc_pid_maps_operations = {
	.open		= pid_maps_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= proc_map_release,
};

/*
 * Proportional Set Size(PSS): my share of RSS.
 *
 * PSS of a process is the count of pages it has in memory, where each
 * page is divided by the number of processes sharing it.  So if a
 * process has 1000 pages all to itself, and 1000 shared with one other
 * process, its PSS will be 1500.
 *
 * To keep (accumulated) division errors low, we adopt a 64bit
 * fixed-point pss counter to minimize division errors. So (pss >>
 * PSS_SHIFT) would be the real byte count.
 *
 * A shift of 12 before division means (assuming 4K page size):
 * 	- 1M 3-user-pages add up to 8KB errors;
 * 	- supports mapcount up to 2^24, or 16M;
 * 	- supports PSS up to 2^52 bytes, or 4PB.
 */
#define PSS_SHIFT 12

#ifdef CONFIG_PROC_PAGE_MONITOR
struct mem_size_stats {
	unsigned long resident;
	unsigned long shared_clean;
	unsigned long shared_dirty;
	unsigned long private_clean;
	unsigned long private_dirty;
	unsigned long referenced;
	unsigned long anonymous;
	unsigned long lazyfree;
	unsigned long anonymous_thp;
	unsigned long shmem_thp;
	unsigned long swap;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
	unsigned long writeback;
#endif
	unsigned long shared_hugetlb;
	unsigned long private_hugetlb;
	u64 pss;
	u64 pss_locked;
	u64 swap_pss;
	bool check_shmem_swap;
};

static void smaps_account(struct mem_size_stats *mss, struct page *page,
		bool compound, bool young, bool dirty, bool locked)
{
	int i, nr = compound ? 1 << compound_order(page) : 1;
	unsigned long size = nr * PAGE_SIZE;

	if (PageAnon(page)) {
		mss->anonymous += size;
		if (!PageSwapBacked(page) && !dirty && !PageDirty(page))
			mss->lazyfree += size;
	}

	mss->resident += size;
	/* Accumulate the size in pages that have been accessed. */
	if (young || page_is_young(page) || PageReferenced(page))
		mss->referenced += size;

	/*
	 * page_count(page) == 1 guarantees the page is mapped exactly once.
	 * If any subpage of the compound page mapped with PTE it would elevate
	 * page_count().
	 */
	if (page_count(page) == 1) {
		if (dirty || PageDirty(page))
			mss->private_dirty += size;
		else
			mss->private_clean += size;
		mss->pss += (u64)size << PSS_SHIFT;
		if (locked)
			mss->pss_locked += (u64)size << PSS_SHIFT;
		return;
	}

	for (i = 0; i < nr; i++, page++) {
		int mapcount = page_mapcount(page);
		unsigned long pss = (PAGE_SIZE << PSS_SHIFT);

		if (mapcount >= 2) {
			if (dirty || PageDirty(page))
				mss->shared_dirty += PAGE_SIZE;
			else
				mss->shared_clean += PAGE_SIZE;
			mss->pss += pss / mapcount;
			if (locked)
				mss->pss_locked += pss / mapcount;
		} else {
			if (dirty || PageDirty(page))
				mss->private_dirty += PAGE_SIZE;
			else
				mss->private_clean += PAGE_SIZE;
			mss->pss += pss;
			if (locked)
				mss->pss_locked += pss;
		}
	}
}

#ifdef CONFIG_SHMEM
static int smaps_pte_hole(unsigned long addr, unsigned long end,
		struct mm_walk *walk)
{
	struct mem_size_stats *mss = walk->private;

	mss->swap += shmem_partial_swap_usage(
			walk->vma->vm_file->f_mapping, addr, end);

	return 0;
}
#endif

static void smaps_pte_entry(pte_t *pte, unsigned long addr,
		struct mm_walk *walk)
{
	struct mem_size_stats *mss = walk->private;
	struct vm_area_struct *vma = walk->vma;
	bool locked = !!(vma->vm_flags & VM_LOCKED);
	struct page *page = NULL;

	if (pte_present(*pte)) {
		page = vm_normal_page(vma, addr, *pte);
	} else if (is_swap_pte(*pte)) {
		swp_entry_t swpent = pte_to_swp_entry(*pte);

		if (!non_swap_entry(swpent)) {
			int mapcount;

			mss->swap += PAGE_SIZE;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
			if (is_writeback_entry(swpent))
				mss->writeback += PAGE_SIZE;
#endif
			mapcount = swp_swapcount(swpent);
			if (mapcount >= 2) {
				u64 pss_delta = (u64)PAGE_SIZE << PSS_SHIFT;

				do_div(pss_delta, mapcount);
				mss->swap_pss += pss_delta;
			} else {
				mss->swap_pss += (u64)PAGE_SIZE << PSS_SHIFT;
			}
		} else if (is_migration_entry(swpent))
			page = migration_entry_to_page(swpent);
		else if (is_device_private_entry(swpent))
			page = device_private_entry_to_page(swpent);
	} else if (unlikely(IS_ENABLED(CONFIG_SHMEM) && mss->check_shmem_swap
							&& pte_none(*pte))) {
		page = find_get_entry(vma->vm_file->f_mapping,
						linear_page_index(vma, addr));
		if (!page)
			return;

		if (radix_tree_exceptional_entry(page))
			mss->swap += PAGE_SIZE;
		else
			put_page(page);

		return;
	}

	if (!page)
		return;

	smaps_account(mss, page, false, pte_young(*pte), pte_dirty(*pte), locked);
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
static void smaps_pmd_entry(pmd_t *pmd, unsigned long addr,
		struct mm_walk *walk)
{
	struct mem_size_stats *mss = walk->private;
	struct vm_area_struct *vma = walk->vma;
	bool locked = !!(vma->vm_flags & VM_LOCKED);
	struct page *page;

	/* FOLL_DUMP will return -EFAULT on huge zero page */
	page = follow_trans_huge_pmd(vma, addr, pmd, FOLL_DUMP);
	if (IS_ERR_OR_NULL(page))
		return;
	if (PageAnon(page))
		mss->anonymous_thp += HPAGE_PMD_SIZE;
	else if (PageSwapBacked(page))
		mss->shmem_thp += HPAGE_PMD_SIZE;
	else if (is_zone_device_page(page))
		/* pass */;
	else
		VM_BUG_ON_PAGE(1, page);
	smaps_account(mss, page, true, pmd_young(*pmd), pmd_dirty(*pmd), locked);
}
#else
static void smaps_pmd_entry(pmd_t *pmd, unsigned long addr,
		struct mm_walk *walk)
{
}
#endif

static int smaps_pte_range(pmd_t *pmd, unsigned long addr, unsigned long end,
			   struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->vma;
	pte_t *pte;
	spinlock_t *ptl;

	ptl = pmd_trans_huge_lock(pmd, vma);
	if (ptl) {
		if (pmd_present(*pmd))
			smaps_pmd_entry(pmd, addr, walk);
		spin_unlock(ptl);
		goto out;
	}

	if (pmd_trans_unstable(pmd))
		goto out;
	/*
	 * The mmap_sem held all the way back in m_start() is what
	 * keeps khugepaged out of here and from collapsing things
	 * in here.
	 */
	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE)
		smaps_pte_entry(pte, addr, walk);
	pte_unmap_unlock(pte - 1, ptl);
out:
	cond_resched();
	return 0;
}

static void show_smap_vma_flags(struct seq_file *m, struct vm_area_struct *vma)
{
	/*
	 * Don't forget to update Documentation/ on changes.
	 */
	static const char mnemonics[BITS_PER_LONG][2] = {
		/*
		 * In case if we meet a flag we don't know about.
		 */
		[0 ... (BITS_PER_LONG-1)] = "??",

		[ilog2(VM_READ)]	= "rd",
		[ilog2(VM_WRITE)]	= "wr",
		[ilog2(VM_EXEC)]	= "ex",
		[ilog2(VM_SHARED)]	= "sh",
		[ilog2(VM_MAYREAD)]	= "mr",
		[ilog2(VM_MAYWRITE)]	= "mw",
		[ilog2(VM_MAYEXEC)]	= "me",
		[ilog2(VM_MAYSHARE)]	= "ms",
		[ilog2(VM_GROWSDOWN)]	= "gd",
		[ilog2(VM_PFNMAP)]	= "pf",
		[ilog2(VM_DENYWRITE)]	= "dw",
#ifdef CONFIG_X86_INTEL_MPX
		[ilog2(VM_MPX)]		= "mp",
#endif
		[ilog2(VM_LOCKED)]	= "lo",
		[ilog2(VM_IO)]		= "io",
		[ilog2(VM_SEQ_READ)]	= "sr",
		[ilog2(VM_RAND_READ)]	= "rr",
		[ilog2(VM_DONTCOPY)]	= "dc",
		[ilog2(VM_DONTEXPAND)]	= "de",
		[ilog2(VM_ACCOUNT)]	= "ac",
		[ilog2(VM_NORESERVE)]	= "nr",
		[ilog2(VM_HUGETLB)]	= "ht",
		[ilog2(VM_ARCH_1)]	= "ar",
		[ilog2(VM_WIPEONFORK)]	= "wf",
		[ilog2(VM_DONTDUMP)]	= "dd",
#ifdef CONFIG_MEM_SOFT_DIRTY
		[ilog2(VM_SOFTDIRTY)]	= "sd",
#endif
		[ilog2(VM_MIXEDMAP)]	= "mm",
		[ilog2(VM_HUGEPAGE)]	= "hg",
		[ilog2(VM_NOHUGEPAGE)]	= "nh",
		[ilog2(VM_MERGEABLE)]	= "mg",
		[ilog2(VM_UFFD_MISSING)]= "um",
		[ilog2(VM_UFFD_WP)]	= "uw",
#ifdef CONFIG_X86_INTEL_MEMORY_PROTECTION_KEYS
		/* These come out via ProtectionKey: */
		[ilog2(VM_PKEY_BIT0)]	= "",
		[ilog2(VM_PKEY_BIT1)]	= "",
		[ilog2(VM_PKEY_BIT2)]	= "",
		[ilog2(VM_PKEY_BIT3)]	= "",
#endif
#ifdef CONFIG_HAVE_ARCH_USERFAULTFD_MINOR
		[ilog2(VM_UFFD_MINOR)]	= "ui",
#endif /* CONFIG_HAVE_ARCH_USERFAULTFD_MINOR */
	};
	size_t i;

	seq_puts(m, "VmFlags: ");
	for (i = 0; i < BITS_PER_LONG; i++) {
		if (!mnemonics[i][0])
			continue;
		if (vma->vm_flags & (1UL << i)) {
			seq_printf(m, "%c%c ",
				   mnemonics[i][0], mnemonics[i][1]);
		}
	}
	seq_putc(m, '\n');
}

#ifdef CONFIG_HUGETLB_PAGE
static int smaps_hugetlb_range(pte_t *pte, unsigned long hmask,
				 unsigned long addr, unsigned long end,
				 struct mm_walk *walk)
{
	struct mem_size_stats *mss = walk->private;
	struct vm_area_struct *vma = walk->vma;
	struct page *page = NULL;

	if (pte_present(*pte)) {
		page = vm_normal_page(vma, addr, *pte);
	} else if (is_swap_pte(*pte)) {
		swp_entry_t swpent = pte_to_swp_entry(*pte);

		if (is_migration_entry(swpent))
			page = migration_entry_to_page(swpent);
		else if (is_device_private_entry(swpent))
			page = device_private_entry_to_page(swpent);
	}
	if (page) {
		if (page_mapcount(page) >= 2 || hugetlb_pmd_shared(pte))
			mss->shared_hugetlb += huge_page_size(hstate_vma(vma));
		else
			mss->private_hugetlb += huge_page_size(hstate_vma(vma));
	}
	return 0;
}
#endif /* HUGETLB_PAGE */

void __weak arch_show_smap(struct seq_file *m, struct vm_area_struct *vma)
{
}

static void smap_gather_stats(struct vm_area_struct *vma,
			     struct mem_size_stats *mss)
{
	struct mm_walk smaps_walk = {
		.pmd_entry = smaps_pte_range,
#ifdef CONFIG_HUGETLB_PAGE
		.hugetlb_entry = smaps_hugetlb_range,
#endif
		.mm = vma->vm_mm,
	};

	smaps_walk.private = mss;

#ifdef CONFIG_SHMEM
	/* In case of smaps_rollup, reset the value from previous vma */
	mss->check_shmem_swap = false;
	if (vma->vm_file && shmem_mapping(vma->vm_file->f_mapping)) {
		/*
		 * For shared or readonly shmem mappings we know that all
		 * swapped out pages belong to the shmem object, and we can
		 * obtain the swap value much more efficiently. For private
		 * writable mappings, we might have COW pages that are
		 * not affected by the parent swapped out pages of the shmem
		 * object, so we have to distinguish them during the page walk.
		 * Unless we know that the shmem object (or the part mapped by
		 * our VMA) has no swapped out pages at all.
		 */
		unsigned long shmem_swapped = shmem_swap_usage(vma);

		if (!shmem_swapped || (vma->vm_flags & VM_SHARED) ||
					!(vma->vm_flags & VM_WRITE)) {
			mss->swap += shmem_swapped;
		} else {
			mss->check_shmem_swap = true;
			smaps_walk.pte_hole = smaps_pte_hole;
		}
	}
#endif
	/* mmap_sem is held in m_start */
	walk_page_vma(vma, &smaps_walk);
}

/* Show the contents common for smaps and smaps_rollup */
static void __show_smap(struct seq_file *m, const struct mem_size_stats *mss)
{
	seq_printf(m,
		   "Rss:            %8lu kB\n"
		   "Pss:            %8lu kB\n"
		   "Shared_Clean:   %8lu kB\n"
		   "Shared_Dirty:   %8lu kB\n"
		   "Private_Clean:  %8lu kB\n"
		   "Private_Dirty:  %8lu kB\n"
		   "Referenced:     %8lu kB\n"
		   "Anonymous:      %8lu kB\n"
		   "LazyFree:       %8lu kB\n"
		   "AnonHugePages:  %8lu kB\n"
		   "ShmemPmdMapped: %8lu kB\n"
		   "Shared_Hugetlb: %8lu kB\n"
		   "Private_Hugetlb: %7lu kB\n"
		   "Swap:           %8lu kB\n"
		   "SwapPss:        %8lu kB\n"
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
		   "Writeback:      %8lu kB\n"
#endif
		   "Locked:         %8lu kB\n",
		   mss->resident >> 10,
		   (unsigned long)(mss->pss >> (10 + PSS_SHIFT)),
		   mss->shared_clean  >> 10,
		   mss->shared_dirty  >> 10,
		   mss->private_clean >> 10,
		   mss->private_dirty >> 10,
		   mss->referenced >> 10,
		   mss->anonymous >> 10,
		   mss->lazyfree >> 10,
		   mss->anonymous_thp >> 10,
		   mss->shmem_thp >> 10,
		   mss->shared_hugetlb >> 10,
		   mss->private_hugetlb >> 10,
		   mss->swap >> 10,
		   (unsigned long)(mss->swap_pss >> (10 + PSS_SHIFT)),
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
		   mss->writeback >> 10,
#endif
		   (unsigned long)(mss->pss_locked >> (10 + PSS_SHIFT)));
}

static int show_smap(struct seq_file *m, void *v)
{
	struct vm_area_struct *vma = v;
	struct mem_size_stats mss;

	memset(&mss, 0, sizeof(mss));

	smap_gather_stats(vma, &mss);

	show_map_vma(m, vma);
	if (vma_get_anon_name(vma)) {
		seq_puts(m, "Name:           ");
		seq_print_vma_name(m, vma);
		seq_putc(m, '\n');
	}

	seq_printf(m,
		   "Size:           %8lu kB\n"
		   "KernelPageSize: %8lu kB\n"
		   "MMUPageSize:    %8lu kB\n",
		   (vma->vm_end - vma->vm_start) >> 10,
		   vma_kernel_pagesize(vma) >> 10,
		   vma_mmu_pagesize(vma) >> 10);

	__show_smap(m, &mss);

	arch_show_smap(m, vma);
	show_smap_vma_flags(m, vma);

	m_cache_vma(m, vma);

	return 0;
}

static int show_smaps_rollup(struct seq_file *m, void *v)
{
	struct proc_maps_private *priv = m->private;
	struct mem_size_stats mss;
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	unsigned long last_vma_end = 0;
	int ret = 0;

	priv->task = get_proc_task(priv->inode);
	if (!priv->task)
		return -ESRCH;

	mm = priv->mm;
	if (!mm || !mmget_not_zero(mm)) {
		ret = -ESRCH;
		goto out_put_task;
	}

	memset(&mss, 0, sizeof(mss));

	down_read(&mm->mmap_sem);
	hold_task_mempolicy(priv);

	for (vma = priv->mm->mmap; vma;) {
		smap_gather_stats(vma, &mss);
		last_vma_end = vma->vm_end;

		/*
		 * Release mmap_sem temporarily if someone wants to
		 * access it for write request.
		 */
		if (rwsem_is_contended(&mm->mmap_sem)) {
			up_read(&mm->mmap_sem);
			down_read(&mm->mmap_sem);

			/*
			 * After dropping the lock, there are three cases to
			 * consider. See the following example for explanation.
			 *
			 *   +------+------+-----------+
			 *   | VMA1 | VMA2 | VMA3      |
			 *   +------+------+-----------+
			 *   |      |      |           |
			 *  4k     8k     16k         400k
			 *
			 * Suppose we drop the lock after reading VMA2 due to
			 * contention, then we get:
			 *
			 *	last_vma_end = 16k
			 *
			 * 1) VMA2 is freed, but VMA3 exists:
			 *
			 *    find_vma(mm, 16k - 1) will return VMA3.
			 *    In this case, just continue from VMA3.
			 *
			 * 2) VMA2 still exists:
			 *
			 *    find_vma(mm, 16k - 1) will return VMA2.
			 *    Iterate the loop like the original one.
			 *
			 * 3) No more VMAs can be found:
			 *
			 *    find_vma(mm, 16k - 1) will return NULL.
			 *    No more things to do, just break.
			 */
			vma = find_vma(mm, last_vma_end - 1);
			/* Case 3 above */
			if (!vma)
				break;

			/* Case 1 above */
			if (vma->vm_start >= last_vma_end)
				continue;
		}
		/* Case 2 above */
		vma = vma->vm_next;
	}

	show_vma_header_prefix(m, priv->mm->mmap->vm_start,
			       last_vma_end, 0, 0, 0, 0);
	seq_pad(m, ' ');
	seq_puts(m, "[rollup]\n");

	__show_smap(m, &mss);

	release_task_mempolicy(priv);
	up_read(&mm->mmap_sem);
	mmput(mm);

out_put_task:
	put_task_struct(priv->task);
	priv->task = NULL;

	return ret;
}

static const struct seq_operations proc_pid_smaps_op = {
	.start	= m_start,
	.next	= m_next,
	.stop	= m_stop,
	.show	= show_smap
};

static int pid_smaps_open(struct inode *inode, struct file *file)
{
	return do_maps_open(inode, file, &proc_pid_smaps_op);
}

static int smaps_rollup_open(struct inode *inode, struct file *file)
{
	int ret;
	struct proc_maps_private *priv;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL_ACCOUNT);
	if (!priv)
		return -ENOMEM;

	ret = single_open(file, show_smaps_rollup, priv);
	if (ret)
		goto out_free;

	priv->inode = inode;
	priv->mm = proc_mem_open(inode, PTRACE_MODE_READ);
	if (IS_ERR(priv->mm)) {
		ret = PTR_ERR(priv->mm);

		single_release(inode, file);
		goto out_free;
	}

	return 0;

out_free:
	kfree(priv);
	return ret;
}

static int smaps_rollup_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = file->private_data;
	struct proc_maps_private *priv = seq->private;

	if (priv->mm)
		mmdrop(priv->mm);

	kfree(priv);
	return single_release(inode, file);
}

const struct file_operations proc_pid_smaps_operations = {
	.open		= pid_smaps_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= proc_map_release,
};

const struct file_operations proc_pid_smaps_rollup_operations = {
	.open		= smaps_rollup_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= smaps_rollup_release,
};

enum clear_refs_types {
	CLEAR_REFS_ALL = 1,
	CLEAR_REFS_ANON,
	CLEAR_REFS_MAPPED,
	CLEAR_REFS_SOFT_DIRTY,
	CLEAR_REFS_MM_HIWATER_RSS,
	CLEAR_REFS_LAST,
};

struct clear_refs_private {
	enum clear_refs_types type;
};

#ifdef CONFIG_MEM_SOFT_DIRTY
static inline void clear_soft_dirty(struct vm_area_struct *vma,
		unsigned long addr, pte_t *pte)
{
	/*
	 * The soft-dirty tracker uses #PF-s to catch writes
	 * to pages, so write-protect the pte as well. See the
	 * Documentation/vm/soft-dirty.txt for full description
	 * of how soft-dirty works.
	 */
	pte_t ptent = *pte;

	if (pte_present(ptent)) {
		ptent = ptep_modify_prot_start(vma->vm_mm, addr, pte);
		ptent = pte_wrprotect(ptent);
		ptent = pte_clear_soft_dirty(ptent);
		ptep_modify_prot_commit(vma->vm_mm, addr, pte, ptent);
	} else if (is_swap_pte(ptent)) {
		ptent = pte_swp_clear_soft_dirty(ptent);
		set_pte_at(vma->vm_mm, addr, pte, ptent);
	}
}
#else
static inline void clear_soft_dirty(struct vm_area_struct *vma,
		unsigned long addr, pte_t *pte)
{
}
#endif

#if defined(CONFIG_MEM_SOFT_DIRTY) && defined(CONFIG_TRANSPARENT_HUGEPAGE)
static inline void clear_soft_dirty_pmd(struct vm_area_struct *vma,
		unsigned long addr, pmd_t *pmdp)
{
	pmd_t pmd = *pmdp;

	if (pmd_present(pmd)) {
		/* See comment in change_huge_pmd() */
		pmdp_invalidate(vma, addr, pmdp);
		if (pmd_dirty(*pmdp))
			pmd = pmd_mkdirty(pmd);
		if (pmd_young(*pmdp))
			pmd = pmd_mkyoung(pmd);

		pmd = pmd_wrprotect(pmd);
		pmd = pmd_clear_soft_dirty(pmd);

		set_pmd_at(vma->vm_mm, addr, pmdp, pmd);
	} else if (is_migration_entry(pmd_to_swp_entry(pmd))) {
		pmd = pmd_swp_clear_soft_dirty(pmd);
		set_pmd_at(vma->vm_mm, addr, pmdp, pmd);
	}
}
#else
static inline void clear_soft_dirty_pmd(struct vm_area_struct *vma,
		unsigned long addr, pmd_t *pmdp)
{
}
#endif

static int clear_refs_pte_range(pmd_t *pmd, unsigned long addr,
				unsigned long end, struct mm_walk *walk)
{
	struct clear_refs_private *cp = walk->private;
	struct vm_area_struct *vma = walk->vma;
	pte_t *pte, ptent;
	spinlock_t *ptl;
	struct page *page;

	ptl = pmd_trans_huge_lock(pmd, vma);
	if (ptl) {
		if (cp->type == CLEAR_REFS_SOFT_DIRTY) {
			clear_soft_dirty_pmd(vma, addr, pmd);
			goto out;
		}

		if (!pmd_present(*pmd))
			goto out;

		page = pmd_page(*pmd);

		/* Clear accessed and referenced bits. */
		pmdp_test_and_clear_young(vma, addr, pmd);
		test_and_clear_page_young(page);
		ClearPageReferenced(page);
out:
		spin_unlock(ptl);
		return 0;
	}

	if (pmd_trans_unstable(pmd))
		return 0;

	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		ptent = *pte;

		if (cp->type == CLEAR_REFS_SOFT_DIRTY) {
			clear_soft_dirty(vma, addr, pte);
			continue;
		}

		if (!pte_present(ptent))
			continue;

		page = vm_normal_page(vma, addr, ptent);
		if (!page)
			continue;

		/* Clear accessed and referenced bits. */
		ptep_test_and_clear_young(vma, addr, pte);
		test_and_clear_page_young(page);
		ClearPageReferenced(page);
	}
	pte_unmap_unlock(pte - 1, ptl);
	cond_resched();
	return 0;
}

static int clear_refs_test_walk(unsigned long start, unsigned long end,
				struct mm_walk *walk)
{
	struct clear_refs_private *cp = walk->private;
	struct vm_area_struct *vma = walk->vma;

	if (vma->vm_flags & VM_PFNMAP)
		return 1;

	/*
	 * Writing 1 to /proc/pid/clear_refs affects all pages.
	 * Writing 2 to /proc/pid/clear_refs only affects anonymous pages.
	 * Writing 3 to /proc/pid/clear_refs only affects file mapped pages.
	 * Writing 4 to /proc/pid/clear_refs affects all pages.
	 */
	if (cp->type == CLEAR_REFS_ANON && vma->vm_file)
		return 1;
	if (cp->type == CLEAR_REFS_MAPPED && !vma->vm_file)
		return 1;
	return 0;
}

static ssize_t clear_refs_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	struct task_struct *task;
	char buffer[PROC_NUMBUF];
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	enum clear_refs_types type;
	struct mmu_gather tlb;
	int itype;
	int rv;

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;
	if (copy_from_user(buffer, buf, count))
		return -EFAULT;
	rv = kstrtoint(strstrip(buffer), 10, &itype);
	if (rv < 0)
		return rv;
	type = (enum clear_refs_types)itype;
	if (type < CLEAR_REFS_ALL || type >= CLEAR_REFS_LAST)
		return -EINVAL;

	task = get_proc_task(file_inode(file));
	if (!task)
		return -ESRCH;
	mm = get_task_mm(task);
	if (mm) {
		struct clear_refs_private cp = {
			.type = type,
		};
		struct mm_walk clear_refs_walk = {
			.pmd_entry = clear_refs_pte_range,
			.test_walk = clear_refs_test_walk,
			.mm = mm,
			.private = &cp,
		};

		if (type == CLEAR_REFS_MM_HIWATER_RSS) {
			if (down_write_killable(&mm->mmap_sem)) {
				count = -EINTR;
				goto out_mm;
			}

			/*
			 * Writing 5 to /proc/pid/clear_refs resets the peak
			 * resident set size to this mm's current rss value.
			 */
			reset_mm_hiwater_rss(mm);
			up_write(&mm->mmap_sem);
			goto out_mm;
		}

		down_read(&mm->mmap_sem);
		tlb_gather_mmu(&tlb, mm, 0, -1);
		if (type == CLEAR_REFS_SOFT_DIRTY) {
			for (vma = mm->mmap; vma; vma = vma->vm_next) {
				if (!(vma->vm_flags & VM_SOFTDIRTY))
					continue;
				up_read(&mm->mmap_sem);
				if (down_write_killable(&mm->mmap_sem)) {
					count = -EINTR;
					goto out_mm;
				}
				/*
				 * Avoid to modify vma->vm_flags
				 * without locked ops while the
				 * coredump reads the vm_flags.
				 */
				if (!mmget_still_valid(mm)) {
					/*
					 * Silently return "count"
					 * like if get_task_mm()
					 * failed. FIXME: should this
					 * function have returned
					 * -ESRCH if get_task_mm()
					 * failed like if
					 * get_proc_task() fails?
					 */
					up_write(&mm->mmap_sem);
					goto out_mm;
				}
				for (vma = mm->mmap; vma; vma = vma->vm_next) {
					vm_write_begin(vma);
					WRITE_ONCE(vma->vm_flags,
						   vma->vm_flags &
						   ~VM_SOFTDIRTY);
					vma_set_page_prot(vma);
					vm_write_end(vma);
				}
				downgrade_write(&mm->mmap_sem);
				break;
			}
			mmu_notifier_invalidate_range_start(mm, 0, -1);
		}
		walk_page_range(0, mm->highest_vm_end, &clear_refs_walk);
		if (type == CLEAR_REFS_SOFT_DIRTY)
			mmu_notifier_invalidate_range_end(mm, 0, -1);
		tlb_finish_mmu(&tlb, 0, -1);
		up_read(&mm->mmap_sem);
out_mm:
		mmput(mm);
	}
	put_task_struct(task);

	return count;
}

const struct file_operations proc_clear_refs_operations = {
	.write		= clear_refs_write,
	.llseek		= noop_llseek,
};

typedef struct {
	u64 pme;
} pagemap_entry_t;

struct pagemapread {
	int pos, len;		/* units: PM_ENTRY_BYTES, not bytes */
	pagemap_entry_t *buffer;
	bool show_pfn;
};

#define PAGEMAP_WALK_SIZE	(PMD_SIZE)
#define PAGEMAP_WALK_MASK	(PMD_MASK)

#define PM_ENTRY_BYTES		sizeof(pagemap_entry_t)
#define PM_PFRAME_BITS		55
#define PM_PFRAME_MASK		GENMASK_ULL(PM_PFRAME_BITS - 1, 0)
#define PM_SOFT_DIRTY		BIT_ULL(55)
#define PM_MMAP_EXCLUSIVE	BIT_ULL(56)
#define PM_FILE			BIT_ULL(61)
#define PM_SWAP			BIT_ULL(62)
#define PM_PRESENT		BIT_ULL(63)

#define PM_END_OF_BUFFER    1

static inline pagemap_entry_t make_pme(u64 frame, u64 flags)
{
	return (pagemap_entry_t) { .pme = (frame & PM_PFRAME_MASK) | flags };
}

static int add_to_pagemap(unsigned long addr, pagemap_entry_t *pme,
			  struct pagemapread *pm)
{
	pm->buffer[pm->pos++] = *pme;
	if (pm->pos >= pm->len)
		return PM_END_OF_BUFFER;
	return 0;
}

static int pagemap_pte_hole(unsigned long start, unsigned long end,
				struct mm_walk *walk)
{
	struct pagemapread *pm = walk->private;
	unsigned long addr = start;
	int err = 0;

	while (addr < end) {
		struct vm_area_struct *vma = find_vma(walk->mm, addr);
		pagemap_entry_t pme = make_pme(0, 0);
		/* End of address space hole, which we mark as non-present. */
		unsigned long hole_end;

		if (vma)
			hole_end = min(end, vma->vm_start);
		else
			hole_end = end;

		for (; addr < hole_end; addr += PAGE_SIZE) {
			err = add_to_pagemap(addr, &pme, pm);
			if (err)
				goto out;
		}

		if (!vma)
			break;

		/* Addresses in the VMA. */
		if (vma->vm_flags & VM_SOFTDIRTY)
			pme = make_pme(0, PM_SOFT_DIRTY);
		for (; addr < min(end, vma->vm_end); addr += PAGE_SIZE) {
			err = add_to_pagemap(addr, &pme, pm);
			if (err)
				goto out;
		}
	}
out:
	return err;
}

static pagemap_entry_t pte_to_pagemap_entry(struct pagemapread *pm,
		struct vm_area_struct *vma, unsigned long addr, pte_t pte)
{
	u64 frame = 0, flags = 0;
	struct page *page = NULL;

	if (pte_present(pte)) {
		if (pm->show_pfn)
			frame = pte_pfn(pte);
		flags |= PM_PRESENT;
		page = _vm_normal_page(vma, addr, pte, true);
		if (pte_soft_dirty(pte))
			flags |= PM_SOFT_DIRTY;
	} else if (is_swap_pte(pte)) {
		swp_entry_t entry;
		if (pte_swp_soft_dirty(pte))
			flags |= PM_SOFT_DIRTY;
		entry = pte_to_swp_entry(pte);
		if (pm->show_pfn)
			frame = swp_type(entry) |
				(swp_offset(entry) << MAX_SWAPFILES_SHIFT);
		flags |= PM_SWAP;
		if (is_migration_entry(entry))
			page = migration_entry_to_page(entry);

		if (is_device_private_entry(entry))
			page = device_private_entry_to_page(entry);
	}

	if (page && !PageAnon(page))
		flags |= PM_FILE;
	if (page && page_mapcount(page) == 1)
		flags |= PM_MMAP_EXCLUSIVE;
	if (vma->vm_flags & VM_SOFTDIRTY)
		flags |= PM_SOFT_DIRTY;

	return make_pme(frame, flags);
}

static int pagemap_pmd_range(pmd_t *pmdp, unsigned long addr, unsigned long end,
			     struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->vma;
	struct pagemapread *pm = walk->private;
	spinlock_t *ptl;
	pte_t *pte, *orig_pte;
	int err = 0;

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	ptl = pmd_trans_huge_lock(pmdp, vma);
	if (ptl) {
		u64 flags = 0, frame = 0;
		pmd_t pmd = *pmdp;
		struct page *page = NULL;

		if (vma->vm_flags & VM_SOFTDIRTY)
			flags |= PM_SOFT_DIRTY;

		if (pmd_present(pmd)) {
			page = pmd_page(pmd);

			flags |= PM_PRESENT;
			if (pmd_soft_dirty(pmd))
				flags |= PM_SOFT_DIRTY;
			if (pm->show_pfn)
				frame = pmd_pfn(pmd) +
					((addr & ~PMD_MASK) >> PAGE_SHIFT);
		}
#ifdef CONFIG_ARCH_ENABLE_THP_MIGRATION
		else if (is_swap_pmd(pmd)) {
			swp_entry_t entry = pmd_to_swp_entry(pmd);
			unsigned long offset;

			if (pm->show_pfn) {
				offset = swp_offset(entry) +
					((addr & ~PMD_MASK) >> PAGE_SHIFT);
				frame = swp_type(entry) |
					(offset << MAX_SWAPFILES_SHIFT);
			}
			flags |= PM_SWAP;
			if (pmd_swp_soft_dirty(pmd))
				flags |= PM_SOFT_DIRTY;
			VM_BUG_ON(!is_pmd_migration_entry(pmd));
			page = migration_entry_to_page(entry);
		}
#endif

		if (page && page_mapcount(page) == 1)
			flags |= PM_MMAP_EXCLUSIVE;

		for (; addr != end; addr += PAGE_SIZE) {
			pagemap_entry_t pme = make_pme(frame, flags);

			err = add_to_pagemap(addr, &pme, pm);
			if (err)
				break;
			if (pm->show_pfn) {
				if (flags & PM_PRESENT)
					frame++;
				else if (flags & PM_SWAP)
					frame += (1 << MAX_SWAPFILES_SHIFT);
			}
		}
		spin_unlock(ptl);
		return err;
	}

	if (pmd_trans_unstable(pmdp))
		return 0;
#endif /* CONFIG_TRANSPARENT_HUGEPAGE */

	/*
	 * We can assume that @vma always points to a valid one and @end never
	 * goes beyond vma->vm_end.
	 */
	orig_pte = pte = pte_offset_map_lock(walk->mm, pmdp, addr, &ptl);
	for (; addr < end; pte++, addr += PAGE_SIZE) {
		pagemap_entry_t pme;

		pme = pte_to_pagemap_entry(pm, vma, addr, *pte);
		err = add_to_pagemap(addr, &pme, pm);
		if (err)
			break;
	}
	pte_unmap_unlock(orig_pte, ptl);

	cond_resched();

	return err;
}

#ifdef CONFIG_HUGETLB_PAGE
/* This function walks within one hugetlb entry in the single call */
static int pagemap_hugetlb_range(pte_t *ptep, unsigned long hmask,
				 unsigned long addr, unsigned long end,
				 struct mm_walk *walk)
{
	struct pagemapread *pm = walk->private;
	struct vm_area_struct *vma = walk->vma;
	u64 flags = 0, frame = 0;
	int err = 0;
	pte_t pte;

	if (vma->vm_flags & VM_SOFTDIRTY)
		flags |= PM_SOFT_DIRTY;

	pte = huge_ptep_get(ptep);
	if (pte_present(pte)) {
		struct page *page = pte_page(pte);

		if (!PageAnon(page))
			flags |= PM_FILE;

		if (page_mapcount(page) == 1)
			flags |= PM_MMAP_EXCLUSIVE;

		flags |= PM_PRESENT;
		if (pm->show_pfn)
			frame = pte_pfn(pte) +
				((addr & ~hmask) >> PAGE_SHIFT);
	}

	for (; addr != end; addr += PAGE_SIZE) {
		pagemap_entry_t pme = make_pme(frame, flags);

		err = add_to_pagemap(addr, &pme, pm);
		if (err)
			return err;
		if (pm->show_pfn && (flags & PM_PRESENT))
			frame++;
	}

	cond_resched();

	return err;
}
#endif /* HUGETLB_PAGE */

/*
 * /proc/pid/pagemap - an array mapping virtual pages to pfns
 *
 * For each page in the address space, this file contains one 64-bit entry
 * consisting of the following:
 *
 * Bits 0-54  page frame number (PFN) if present
 * Bits 0-4   swap type if swapped
 * Bits 5-54  swap offset if swapped
 * Bit  55    pte is soft-dirty (see Documentation/vm/soft-dirty.txt)
 * Bit  56    page exclusively mapped
 * Bits 57-60 zero
 * Bit  61    page is file-page or shared-anon
 * Bit  62    page swapped
 * Bit  63    page present
 *
 * If the page is not present but in swap, then the PFN contains an
 * encoding of the swap file number and the page's offset into the
 * swap. Unmapped pages return a null PFN. This allows determining
 * precisely which pages are mapped (or in swap) and comparing mapped
 * pages between processes.
 *
 * Efficient users of this interface will use /proc/pid/maps to
 * determine which areas of memory are actually mapped and llseek to
 * skip over unmapped regions.
 */
static ssize_t pagemap_read(struct file *file, char __user *buf,
			    size_t count, loff_t *ppos)
{
	struct mm_struct *mm = file->private_data;
	struct pagemapread pm;
	struct mm_walk pagemap_walk = {};
	unsigned long src;
	unsigned long svpfn;
	unsigned long start_vaddr;
	unsigned long end_vaddr;
	int ret = 0, copied = 0;

	if (!mm || !mmget_not_zero(mm))
		goto out;

	ret = -EINVAL;
	/* file position must be aligned */
	if ((*ppos % PM_ENTRY_BYTES) || (count % PM_ENTRY_BYTES))
		goto out_mm;

	ret = 0;
	if (!count)
		goto out_mm;

	/* do not disclose physical addresses: attack vector */
	pm.show_pfn = file_ns_capable(file, &init_user_ns, CAP_SYS_ADMIN);

	pm.len = (PAGEMAP_WALK_SIZE >> PAGE_SHIFT);
	pm.buffer = kmalloc(pm.len * PM_ENTRY_BYTES, GFP_KERNEL);
	ret = -ENOMEM;
	if (!pm.buffer)
		goto out_mm;

	pagemap_walk.pmd_entry = pagemap_pmd_range;
	pagemap_walk.pte_hole = pagemap_pte_hole;
#ifdef CONFIG_HUGETLB_PAGE
	pagemap_walk.hugetlb_entry = pagemap_hugetlb_range;
#endif
	pagemap_walk.mm = mm;
	pagemap_walk.private = &pm;

	src = *ppos;
	svpfn = src / PM_ENTRY_BYTES;
	start_vaddr = untagged_addr(svpfn << PAGE_SHIFT);
	end_vaddr = mm->task_size;

	/* watch out for wraparound */
	if (start_vaddr > mm->task_size)
		start_vaddr = end_vaddr;

	/*
	 * The odds are that this will stop walking way
	 * before end_vaddr, because the length of the
	 * user buffer is tracked in "pm", and the walk
	 * will stop when we hit the end of the buffer.
	 */
	ret = 0;
	while (count && (start_vaddr < end_vaddr)) {
		int len;
		unsigned long end;

		pm.pos = 0;
		end = (start_vaddr + PAGEMAP_WALK_SIZE) & PAGEMAP_WALK_MASK;
		/* overflow ? */
		if (end < start_vaddr || end > end_vaddr)
			end = end_vaddr;
		down_read(&mm->mmap_sem);
		ret = walk_page_range(start_vaddr, end, &pagemap_walk);
		up_read(&mm->mmap_sem);
		start_vaddr = end;

		len = min(count, PM_ENTRY_BYTES * pm.pos);
		if (copy_to_user(buf, pm.buffer, len)) {
			ret = -EFAULT;
			goto out_free;
		}
		copied += len;
		buf += len;
		count -= len;
	}
	*ppos += copied;
	if (!ret || ret == PM_END_OF_BUFFER)
		ret = copied;

out_free:
	kfree(pm.buffer);
out_mm:
	mmput(mm);
out:
	return ret;
}

static int pagemap_open(struct inode *inode, struct file *file)
{
	struct mm_struct *mm;

	mm = proc_mem_open(inode, PTRACE_MODE_READ);
	if (IS_ERR(mm))
		return PTR_ERR(mm);
	file->private_data = mm;
	return 0;
}

static int pagemap_release(struct inode *inode, struct file *file)
{
	struct mm_struct *mm = file->private_data;

	if (mm)
		mmdrop(mm);
	return 0;
}

const struct file_operations proc_pagemap_operations = {
	.llseek		= mem_lseek, /* borrow this */
	.read		= pagemap_read,
	.open		= pagemap_open,
	.release	= pagemap_release,
};
#endif /* CONFIG_PROC_PAGE_MONITOR */

#ifdef CONFIG_FREEZING
static inline bool is_pm_freezing(void)
{
	return pm_freezing;
}
#else
static inline bool is_pm_freezing(void)
{
	return false;
}
#endif /* CONFIG_FREEZING */

#ifdef CONFIG_PROCESS_RECLAIM
static int deactivate_pte_range(pmd_t *pmd, unsigned long addr,
				unsigned long end, struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->private;
	pte_t *pte, ptent;
	spinlock_t *ptl;
	struct page *page;

	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr < end; pte++, addr += PAGE_SIZE) {
		ptent = *pte;

		if (pte_none(ptent))
			continue;

		if (!pte_present(ptent))
			continue;

		page = vm_normal_page(vma, addr, ptent);
		if (!page)
			continue;
		/*
		 * XXX: we don't handle compound page at this moment but
		 * it should revisit for THP page before upstream.
		 */
		if (PageCompound(page)) {
			unsigned int order = compound_order(page);
			unsigned int nr_pages = (1 << order) - 1;

			addr += (nr_pages * PAGE_SIZE);
			pte += nr_pages;
			continue;
		}

		if (page_mapcount(page) > 1)
			continue;

		ptep_test_and_clear_young(vma, addr, pte);
		test_and_clear_page_young(page);
		if (PageReferenced(page))
			ClearPageReferenced(page);
		if (PageActive(page))
			deactivate_file_page(page);
	}
	pte_unmap_unlock(pte - 1, ptl);

	cond_resched();
	return 0;
}

static int reclaim_pte_range(pmd_t *pmd, unsigned long addr,
				unsigned long end, struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->private;
	pte_t *pte, ptent;
	spinlock_t *ptl;
	struct page *page;
	LIST_HEAD(page_list);
	int isolated;

#ifdef CONFIG_ZRAM_LRU_WRITEBACK
	bool is_lru_wb = false;

	if (!strcmp("PerProcessNands", current->comm))
		is_lru_wb = true;
#endif

	split_huge_pmd(vma, addr, pmd);
	if (pmd_trans_unstable(pmd))
		return 0;
cont:
	if (rwsem_is_contended(&walk->mm->mmap_sem))
		return -1;
	if (is_pm_freezing())
		return -1;

	isolated = 0;
	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		ptent = *pte;
		if (!pte_present(ptent))
			continue;

		page = vm_normal_page(vma, addr, ptent);
		if (!page)
			continue;

		if (PageUnevictable(page))
			continue;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
		if (is_lru_wb && ptep_test_and_clear_young(vma, addr, pte))
			continue;
#endif

		if (!PageLRU(page))
			continue;

		if (isolate_lru_page(page))
			continue;

		list_add(&page->lru, &page_list);
		isolated++;
		if (isolated >= SWAP_CLUSTER_MAX)
			break;
	}
	pte_unmap_unlock(pte - 1, ptl);
	reclaim_pages_from_list(&page_list, vma);
	if (addr != end)
		goto cont;

	cond_resched();
	return 0;
}

#ifdef CONFIG_ZRAM_LRU_WRITEBACK
static int writeback_pte_range(pmd_t *pmd, unsigned long addr,
		unsigned long end, struct mm_walk *walk)
{
	struct mm_struct *mm = walk->mm;
	struct zwbs **zwbs = walk->private;
	pte_t *pte, ptent;
	spinlock_t *ptl;
	LIST_HEAD(swp_entry_list);

	if (pmd_trans_unstable(pmd))
		return 0;
	if (rwsem_is_contended(&mm->mmap_sem))
		return -1;
	if (is_pm_freezing())
		return -1;
	if (zram_is_app_launch())
		return -EBUSY;

	pte = pte_offset_map_lock(mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		ptent = *pte;
		if (is_swap_pte(ptent)) {
			swp_entry_t entry = pte_to_swp_entry(ptent);
			if (unlikely(non_swap_entry(entry)))
				continue;
			if (swp_swapcount(entry) > 1)
				continue;
			swap_add_to_list(&swp_entry_list, entry);
		}
	}
	pte_unmap_unlock(pte - 1, ptl);
	swap_writeback_list(zwbs, &swp_entry_list);

	cond_resched();
	return 0;
}
#endif
enum reclaim_type {
	RECLAIM_FILE,
	RECLAIM_ANON,
	RECLAIM_ALL,
	RECLAIM_RANGE,
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
	RECLAIM_WRITEBACK,
#endif
};

static ssize_t reclaim_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	struct task_struct *task;
	char buffer[PROC_NUMBUF];
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	enum reclaim_type type;
	char *type_buf;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
	struct zwbs *zwbs[NR_ZWBS];
#endif
	int err = 0;

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;

	if (copy_from_user(buffer, buf, count))
		return -EFAULT;

	type_buf = strstrip(buffer);
	if (!strcmp(type_buf, "file"))
		type = RECLAIM_FILE;
	else if (!strcmp(type_buf, "anon"))
		type = RECLAIM_ANON;
	else if (!strcmp(type_buf, "all"))
		type = RECLAIM_ALL;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
	else if (!strcmp(type_buf, "writeback"))
		type = RECLAIM_WRITEBACK;
#endif
	else
		return -EINVAL;

	task = get_proc_task(file->f_path.dentry->d_inode);
	if (!task)
		return -ESRCH;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
	if (type == RECLAIM_WRITEBACK) {
		if (alloc_zwbs(zwbs)) {
			pr_info("%s alloc_zwbs failed", __func__);
			return -ENOMEM;
		}
	}
#endif

	mm = get_task_mm(task);
	if (mm) {
		struct mm_walk reclaim_walk = {
			.pmd_entry = reclaim_pte_range,
			.mm = mm,
		};

		down_read(&mm->mmap_sem);
		for (vma = mm->mmap; vma; vma = vma->vm_next) {
			reclaim_walk.private = vma;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
			if (type == RECLAIM_WRITEBACK)
				reclaim_walk.private = (void *)zwbs;
#endif

			if (vma->vm_flags & VM_LOCKED)
				continue;

			if (is_vm_hugetlb_page(vma))
				continue;

			if (type == RECLAIM_ANON && vma->vm_file)
				continue;
			if (type == RECLAIM_FILE && !vma->vm_file)
				continue;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
			if (type == RECLAIM_WRITEBACK && vma->vm_file)
				continue;
#endif

			if (!vma->vm_file)
				reclaim_walk.pmd_entry = reclaim_pte_range;
			else
				reclaim_walk.pmd_entry = deactivate_pte_range;
#ifdef CONFIG_ZRAM_LRU_WRITEBACK
			if (type == RECLAIM_WRITEBACK)
				reclaim_walk.pmd_entry = writeback_pte_range;
#endif

			err = walk_page_range(vma->vm_start, vma->vm_end,
					&reclaim_walk);
			if (err) {
				count = err;
				break;
			}
		}
		flush_tlb_mm(mm);
		up_read(&mm->mmap_sem);
		mmput(mm);
	}
	put_task_struct(task);

#ifdef CONFIG_ZRAM_LRU_WRITEBACK
	if (type == RECLAIM_WRITEBACK) {
		swap_writeback_list(zwbs, NULL);
		free_zwbs(zwbs);
	}
#endif
	return count;
}

const struct file_operations proc_reclaim_operations = {
	.write		= reclaim_write,
	.llseek		= noop_llseek,
};
#endif

#ifdef CONFIG_NUMA

struct numa_maps {
	unsigned long pages;
	unsigned long anon;
	unsigned long active;
	unsigned long writeback;
	unsigned long mapcount_max;
	unsigned long dirty;
	unsigned long swapcache;
	unsigned long node[MAX_NUMNODES];
};

struct numa_maps_private {
	struct proc_maps_private proc_maps;
	struct numa_maps md;
};

static void gather_stats(struct page *page, struct numa_maps *md, int pte_dirty,
			unsigned long nr_pages)
{
	int count = page_mapcount(page);

	md->pages += nr_pages;
	if (pte_dirty || PageDirty(page))
		md->dirty += nr_pages;

	if (PageSwapCache(page))
		md->swapcache += nr_pages;

	if (PageActive(page) || PageUnevictable(page))
		md->active += nr_pages;

	if (PageWriteback(page))
		md->writeback += nr_pages;

	if (PageAnon(page))
		md->anon += nr_pages;

	if (count > md->mapcount_max)
		md->mapcount_max = count;

	md->node[page_to_nid(page)] += nr_pages;
}

static struct page *can_gather_numa_stats(pte_t pte, struct vm_area_struct *vma,
		unsigned long addr)
{
	struct page *page;
	int nid;

	if (!pte_present(pte))
		return NULL;

	page = vm_normal_page(vma, addr, pte);
	if (!page)
		return NULL;

	if (PageReserved(page))
		return NULL;

	nid = page_to_nid(page);
	if (!node_isset(nid, node_states[N_MEMORY]))
		return NULL;

	return page;
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
static struct page *can_gather_numa_stats_pmd(pmd_t pmd,
					      struct vm_area_struct *vma,
					      unsigned long addr)
{
	struct page *page;
	int nid;

	if (!pmd_present(pmd))
		return NULL;

	page = vm_normal_page_pmd(vma, addr, pmd);
	if (!page)
		return NULL;

	if (PageReserved(page))
		return NULL;

	nid = page_to_nid(page);
	if (!node_isset(nid, node_states[N_MEMORY]))
		return NULL;

	return page;
}
#endif

static int gather_pte_stats(pmd_t *pmd, unsigned long addr,
		unsigned long end, struct mm_walk *walk)
{
	struct numa_maps *md = walk->private;
	struct vm_area_struct *vma = walk->vma;
	spinlock_t *ptl;
	pte_t *orig_pte;
	pte_t *pte;

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	ptl = pmd_trans_huge_lock(pmd, vma);
	if (ptl) {
		struct page *page;

		page = can_gather_numa_stats_pmd(*pmd, vma, addr);
		if (page)
			gather_stats(page, md, pmd_dirty(*pmd),
				     HPAGE_PMD_SIZE/PAGE_SIZE);
		spin_unlock(ptl);
		return 0;
	}

	if (pmd_trans_unstable(pmd))
		return 0;
#endif
	orig_pte = pte = pte_offset_map_lock(walk->mm, pmd, addr, &ptl);
	do {
		struct page *page = can_gather_numa_stats(*pte, vma, addr);
		if (!page)
			continue;
		gather_stats(page, md, pte_dirty(*pte), 1);

	} while (pte++, addr += PAGE_SIZE, addr != end);
	pte_unmap_unlock(orig_pte, ptl);
	cond_resched();
	return 0;
}
#ifdef CONFIG_HUGETLB_PAGE
static int gather_hugetlb_stats(pte_t *pte, unsigned long hmask,
		unsigned long addr, unsigned long end, struct mm_walk *walk)
{
	pte_t huge_pte = huge_ptep_get(pte);
	struct numa_maps *md;
	struct page *page;

	if (!pte_present(huge_pte))
		return 0;

	page = pte_page(huge_pte);
	if (!page)
		return 0;

	md = walk->private;
	gather_stats(page, md, pte_dirty(huge_pte), 1);
	return 0;
}

#else
static int gather_hugetlb_stats(pte_t *pte, unsigned long hmask,
		unsigned long addr, unsigned long end, struct mm_walk *walk)
{
	return 0;
}
#endif

/*
 * Display pages allocated per node and memory policy via /proc.
 */
static int show_numa_map(struct seq_file *m, void *v)
{
	struct numa_maps_private *numa_priv = m->private;
	struct proc_maps_private *proc_priv = &numa_priv->proc_maps;
	struct vm_area_struct *vma = v;
	struct numa_maps *md = &numa_priv->md;
	struct file *file = vma->vm_file;
	struct mm_struct *mm = vma->vm_mm;
	struct mm_walk walk = {
		.hugetlb_entry = gather_hugetlb_stats,
		.pmd_entry = gather_pte_stats,
		.private = md,
		.mm = mm,
	};
	struct mempolicy *pol;
	char buffer[64];
	int nid;

	if (!mm)
		return 0;

	/* Ensure we start with an empty set of numa_maps statistics. */
	memset(md, 0, sizeof(*md));

	pol = __get_vma_policy(vma, vma->vm_start);
	if (pol) {
		mpol_to_str(buffer, sizeof(buffer), pol);
		mpol_cond_put(pol);
	} else {
		mpol_to_str(buffer, sizeof(buffer), proc_priv->task_mempolicy);
	}

	seq_printf(m, "%08lx %s", vma->vm_start, buffer);

	if (file) {
		seq_puts(m, " file=");
		seq_file_path(m, file, "\n\t= ");
	} else if (vma->vm_start <= mm->brk && vma->vm_end >= mm->start_brk) {
		seq_puts(m, " heap");
	} else if (is_stack(vma)) {
		seq_puts(m, " stack");
	}

	if (is_vm_hugetlb_page(vma))
		seq_puts(m, " huge");

	/* mmap_sem is held by m_start */
	walk_page_vma(vma, &walk);

	if (!md->pages)
		goto out;

	if (md->anon)
		seq_printf(m, " anon=%lu", md->anon);

	if (md->dirty)
		seq_printf(m, " dirty=%lu", md->dirty);

	if (md->pages != md->anon && md->pages != md->dirty)
		seq_printf(m, " mapped=%lu", md->pages);

	if (md->mapcount_max > 1)
		seq_printf(m, " mapmax=%lu", md->mapcount_max);

	if (md->swapcache)
		seq_printf(m, " swapcache=%lu", md->swapcache);

	if (md->active < md->pages && !is_vm_hugetlb_page(vma))
		seq_printf(m, " active=%lu", md->active);

	if (md->writeback)
		seq_printf(m, " writeback=%lu", md->writeback);

	for_each_node_state(nid, N_MEMORY)
		if (md->node[nid])
			seq_printf(m, " N%d=%lu", nid, md->node[nid]);

	seq_printf(m, " kernelpagesize_kB=%lu", vma_kernel_pagesize(vma) >> 10);
out:
	seq_putc(m, '\n');
	m_cache_vma(m, vma);
	return 0;
}

static const struct seq_operations proc_pid_numa_maps_op = {
	.start  = m_start,
	.next   = m_next,
	.stop   = m_stop,
	.show   = show_numa_map,
};

static int pid_numa_maps_open(struct inode *inode, struct file *file)
{
	return proc_maps_open(inode, file, &proc_pid_numa_maps_op,
				sizeof(struct numa_maps_private));
}

const struct file_operations proc_pid_numa_maps_operations = {
	.open		= pid_numa_maps_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= proc_map_release,
};

#endif /* CONFIG_NUMA */

#ifdef CONFIG_PAGE_BOOST
/*
 * Currently, target_file_name is shared by all filemap_info nodes
 * as we do not access this node in parallel. (do not need synchronization also)
 */
#include <linux/io_record.h>
#include <linux/atomic.h>
static atomic_t filemap_fd_opened = ATOMIC_INIT(0);
char target_file_name[MAX_PAGE_BOOST_FILEPATH_LEN + 1] = "";

static inline bool try_to_get_filemap_fd(void)
{
	/* only 1 context is allowed at a time */
	if (atomic_inc_return(&filemap_fd_opened) == 1)
		return true;
	else {
		atomic_dec(&filemap_fd_opened);
		return false;
	}
}

static inline void put_filemap_fd(void)
{
	atomic_dec(&filemap_fd_opened);
}

static void
show_filemap_vma(struct seq_file *m, struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct proc_filemap_private *priv = m->private;
	char strbuf[MAX_PAGE_BOOST_FILEPATH_LEN];
	char *pathname;

	if (!file)
		return;

	pathname = d_path(&file->f_path, strbuf, MAX_PAGE_BOOST_FILEPATH_LEN);
	if (IS_ERR(pathname))
		return;

	if (priv->show_list) {
		if (!strncmp(pathname, "/data", 5) ||
		    !strncmp(pathname, "/system", 7)) {
			seq_puts(m, pathname);
			seq_putc(m, '\n');
		}
	}
}

static int show_filemap(struct seq_file *m, void *v)
{
	show_filemap_vma(m, v);
	m_cache_vma(m, v);
	return 0;
}

static const struct seq_operations proc_pid_filemap_op = {
	.start	= m_start,
	.next	= m_next,
	.stop	= m_stop,
	.show	= show_filemap,
};

static int pid_filemap_list_open(struct inode *inode, struct file *file)
{
	int psize = sizeof(struct proc_filemap_private);
	const struct seq_operations *ops = &proc_pid_filemap_op;
	struct proc_filemap_private *priv = __seq_open_private(file, ops,
							       psize);

	if (!priv)
		return -ENOMEM;
	if (!try_to_get_filemap_fd())
		return -EINVAL;

	priv->maps_private.inode = inode;
	priv->maps_private.mm = proc_mem_open(inode, PTRACE_MODE_READ);
	priv->show_list = true;
	if (IS_ERR(priv->maps_private.mm)) {
		int err = PTR_ERR(priv->maps_private.mm);

		put_filemap_fd();
		seq_release_private(inode, file);
		return err;
	}

	return 0;
}

/* common release for filemap_list and filemap_info */
static int proc_filemap_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = file->private_data;
	struct proc_filemap_private *priv = seq->private;

	if (priv->maps_private.mm)
		mmdrop(priv->maps_private.mm);

	put_filemap_fd();
	return seq_release_private(inode, file);
}

/* List mapped files for this process */
const struct file_operations proc_pid_filemap_list_operations = {
	.open		= pid_filemap_list_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= proc_filemap_release,
};

#ifdef CONFIG_PAGE_BOOST_RECORDING
static ssize_t pid_io_record_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	return read_record(buf, count, ppos);
}

static ssize_t pid_io_record_write(struct file *file,
					       const char __user *buf,
					       size_t count, loff_t *ppos)
{
	char buffer[PROC_NUMBUF];
	int itype;
	enum io_record_cmd_types type;
	int rv;
	struct task_struct *task;
	bool ret = true;

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;
	if (copy_from_user(buffer, buf, count))
		return -EFAULT;
	rv = kstrtoint(strstrip(buffer), 10, &itype);
	if (rv < 0)
		return rv;

	task = get_proc_task(file_inode(file));
	if (!task)
		return -EFAULT;

	type = (enum io_record_cmd_types)itype;
	if (type < IO_RECORD_INIT || type > IO_RECORD_POST_PROCESSING) {
		put_task_struct(task);
		return -EINVAL;
	}

	switch (type) {
	case IO_RECORD_INIT:
		ret = init_record();
		break;
	case IO_RECORD_START:
		ret = start_record((int)task_pid_nr(task));
		break;
	case IO_RECORD_STOP:
		ret = stop_record();
		break;
	case IO_RECORD_POST_PROCESSING:
		ret = post_processing_records();
		break;
	default:
		break;
	}
	put_task_struct(task);

	if (!ret)
		count = -EINVAL;

	return count;
}

const struct file_operations proc_pid_io_record_operations = {
	.read		= pid_io_record_read,
	.write		= pid_io_record_write,
	.llseek		= noop_llseek,
};
#endif
#endif