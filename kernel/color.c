#include <linux/syscalls.h>
#include <linux/colormask.h>

long set_color(pid_t pid, const struct cpumask *in_mask)
{
	return 0;
}

long get_color(pid_t pid, struct cpumask *mask)
{
	return 0;
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
	/*
	cpumask_var_t new_mask;
	int retval;

	if (!alloc_cpumask_var(&new_mask, GFP_KERNEL))
		return -ENOMEM;

	retval = get_user_cpu_mask(user_mask_ptr, len, new_mask);
	if (retval == 0)
		retval = sched_setaffinity(pid, new_mask);
	free_cpumask_var(new_mask);
	*/
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
	/*
	int ret;
	cpumask_var_t mask;

	if ((len * BITS_PER_BYTE) < nr_cpu_ids)
		return -EINVAL;
	if (len & (sizeof(unsigned long)-1))
		return -EINVAL;

	if (!alloc_cpumask_var(&mask, GFP_KERNEL))
		return -ENOMEM;

	ret = sched_getaffinity(pid, mask);
	if (ret == 0) {
		size_t retlen = min_t(size_t, len, cpumask_size());

		if (copy_to_user(user_mask_ptr, mask, retlen))
			ret = -EFAULT;
		else
			ret = retlen;
	}
	free_cpumask_var(mask);
	*/
	return 0;
}

