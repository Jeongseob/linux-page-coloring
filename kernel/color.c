#include <linux/syscalls.h>
#include <linux/colormask.h>
#include <linux/kernel.h>
#include <linux/threads.h>

const DECLARE_BITMAP(color_all_bits, NR_COLORS) = COLOR_BITS_ALL;
EXPORT_SYMBOL(color_all_bits);

static struct task_struct *find_process_by_pid(pid_t pid)
{
	return pid ? find_task_by_vpid(pid) : current;
}

void do_set_colors_allowed(struct task_struct *p, const struct colormask *new_mask)
{
	colormask_copy(&p->colors_allowed, new_mask);
}

int set_colors_allowed_ptr(struct task_struct *p, const struct colormask *new_mask)
{
	int ret = 0;

	do_set_colors_allowed(p, new_mask);
	p->last_color = colormask_first(new_mask);
		
	return ret;
}

long set_color(pid_t pid, const struct colormask *in_mask)
{
	struct task_struct *p;
	int retval;

	p = find_process_by_pid(pid);
	if (!p) {
		return -ESRCH;
	}

	retval = set_colors_allowed_ptr(p, in_mask);

	return retval;
}

long get_color(pid_t pid, struct colormask *mask)
{
	struct task_struct *p;
	int retval = 0;

	p = find_process_by_pid(pid);
	if (!p) {
		retval = -ESRCH;
		goto out;
	}
	
	colormask_copy(mask, &p->colors_allowed);

out:
	return retval;
}

static int get_user_color_mask(unsigned long __user *user_mask_ptr, unsigned len, struct colormask *new_mask)
{
	if (len < colormask_size())
		colormask_clear(new_mask);
	else if (len > colormask_size())
		len = colormask_size();

	return copy_from_user(new_mask, user_mask_ptr, len) ? -EFAULT : 0;
}

/**
 * sys_set_color - set the page color of a process
 * @pid: pid of the process
 * @len: length in bytes of the bitmask pointed to by user_mask_ptr
 * @user_mask_ptr: user-space pointer to the new color mask
 *
 * Return: 0 on success. An error code otherwise.
 */
SYSCALL_DEFINE3(set_color, pid_t, pid, unsigned int, len,
		unsigned long __user *, user_mask_ptr)
{
	colormask_var_t new_mask;
	int retval;

	retval = get_user_color_mask(user_mask_ptr, len, new_mask);
	if ( retval == 0 )
		retval = set_color(pid, new_mask);
	
	return 0;
}


/**
 * sys_get_color - get the page color of a process
 * @pid: pid of the process
 * @len: length in bytes of the bitmask pointed to by user_mask_ptr
 * @user_mask_ptr: user-space pointer to hold the current color mask
 *
 * Return: 0 on success. An error code otherwise.
 */
SYSCALL_DEFINE3(get_color, pid_t, pid, unsigned int, len,
		unsigned long __user *, user_mask_ptr)
{
	int ret;
	colormask_var_t mask;

	if ((len * BITS_PER_BYTE) < NR_COLORS)
		return -EINVAL;
	if (len & (sizeof(unsigned long)-1))
		return -EINVAL;
	
	ret = get_color(pid, mask);

	if (ret == 0) {

		size_t retlen = min_t(size_t, len, colormask_size());
		
		if (copy_to_user(user_mask_ptr, mask, retlen)) {
			ret = -EFAULT;
		}
		else {
			ret = retlen;
		}

	}

	return ret;
}

