#ifndef __LINUX_COLORMASK_H
#define __LINUX_COLORMASK_H

#include <linux/kernel.h>
#include <linux/threads.h>
#include <linux/bitmap.h>

#define NR_COLOR_BITS	4	/* FIXME: num of CPUs */

typedef struct colormask { DECLARE_BITMAP(bits, NR_COLOR_BITS); } colormask_t;

#define colormask_bits(maskp) ((maskp)->bits)

#define to_colormask(bitmap)						\
	((struct colormask *)(1 ? (bitmap)				\
			    : (void *)sizeof(__check_is_bitmap(bitmap))))

extern const DECLARE_BITMAP(color_all_bits, NR_COLOR_BITS);
#define color_all_mask to_colormask(color_all_bits)


#if NR_COLOR_BITS <= BITS_PER_LONG
#define COLOR_BITS_ALL	\
{						\
	[BITS_TO_LONGS(NR_COLOR_BITS)-1] = ~COLOR_MASK_LAST_WORD \
}	
#else
#define COLOR_BITS_ALL	\
{						\
	[0 ... BITS_TO_LONGS(NR_COLOR_BITS)-2] = 0UL,	\
	[BITS_TO_LONGS(NR_COLOR_BITS)-1] = ~COLOR_MASK_LAST_WORD \
}
#endif

#define COLOR_MASK_LAST_WORD	BITMAP_LAST_WORD_MASK(NR_COLOR_BITS)

#define for_each_color(color, mask)				\
	for ((color) = -1;				\
		(color) = colormask_next((color), (mask)),	\
		(color) < NR_COLOR_BITS;)


typedef struct colormask colormask_var_t[1];

static inline size_t colormask_size(void)
{
	return BITS_TO_LONGS(NR_COLOR_BITS) * sizeof(long);
}

static inline void colormask_clear(struct colormask *dstp)
{
	bitmap_zero(colormask_bits(dstp), NR_COLOR_BITS);
}

static inline void colormask_copy(struct colormask* dstp, 
				const struct colormask *srcp)
{
	bitmap_copy(colormask_bits(dstp), colormask_bits(srcp), NR_COLOR_BITS);
}

static inline bool colormask_empty(const struct colormask *srcp)
{
	return bitmap_empty(colormask_bits(srcp), NR_COLOR_BITS);
}

static inline unsigned int colormask_next(int n, const struct colormask *srcp)
{
	return find_next_bit(colormask_bits(srcp), NR_COLOR_BITS, n+1);
}


int set_colors_allowed_ptr(struct task_struct *p, const struct colormask *new_mask);

#endif
