#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/quicklist.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <asm/pgtable.h>

static int colorinfo_proc_show(struct seq_file *m, void *v)
{
	int i;
	struct colorinfo ci[NR_COLORS];

	get_colorinfo(ci);

	for (i = 0; i < NR_COLORS; i++) {
		seq_printf(m, "color[%d]: %lu/%lu\n", i, ci[i].total_free_pages, ci[i].total_color_pages);
	}

	return 0;
}

static int colorinfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, colorinfo_proc_show, NULL);
}

static const struct file_operations colorinfo_proc_fops = {
	.open		= colorinfo_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_colorinfo_init(void)
{
	proc_create("colorinfo", 0, NULL, &colorinfo_proc_fops);
	return 0;
}
fs_initcall(proc_colorinfo_init);
