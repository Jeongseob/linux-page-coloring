#ifndef __LINUX_COLORMASK_H
#define __LINUX_COLORMASK_H

#include <linux/kernel.h>
#include <linux/threads.h>
#include <linux/bitmap.h>

#define NR_COLORS	4	/* FIXME: num of CPUs */
#define RESERVE_COLOR_PAGES		262144 * NR_COLORS	// 262144 * 4KB = 1GB

/* FIXME: it will be dependent on processor architectures */
//#define	CACHE_SET_BITS	13
//#define	LINE_SIZE_BITS	6		// cache_line_size()
//#define	PAGE_SIZE_BITS	12
//#define	COLOR_MASK_BITS	CACHE_SET_BITS + LINE_SIZE_BITS - PAGE_SIZE_BITS

typedef struct colormask { DECLARE_BITMAP(bits, NR_COLORS); } colormask_t;

#define colormask_bits(maskp) ((maskp)->bits)

#define to_colormask(bitmap)						\
	((struct colormask *)(1 ? (bitmap)				\
			    : (void *)sizeof(__check_is_bitmap(bitmap))))

extern const DECLARE_BITMAP(color_all_bits, NR_COLORS);
#define color_all_mask to_colormask(color_all_bits)


#if NR_COLORS <= BITS_PER_LONG
#define COLOR_BITS_ALL	\
{						\
	[BITS_TO_LONGS(NR_COLORS)-1] = ~COLOR_MASK_LAST_WORD \
}	
#else
#define COLOR_BITS_ALL	\
{						\
	[0 ... BITS_TO_LONGS(NR_COLORS)-2] = 0UL,	\
	[BITS_TO_LONGS(NR_COLORS)-1] = ~COLOR_MASK_LAST_WORD \
}
#endif

#define COLOR_MASK_LAST_WORD	BITMAP_LAST_WORD_MASK(NR_COLORS)

#define for_each_color(color, mask)				\
	for ((color) = -1;				\
		(color) = colormask_next((color), (mask)),	\
		(color) < NR_COLORS;)

#define for_each_color_from(color, mask)				\
	for (;	\
		(color) = colormask_next((color), (mask)),	\
		(color) < NR_COLORS;)

#define color_isset(color, colormask) test_bit((color), (colormask).bits)

typedef struct colormask colormask_var_t[1];

static inline size_t colormask_size(void)
{
	return BITS_TO_LONGS(NR_COLORS) * sizeof(long);
}

static inline void colormask_clear(struct colormask *dstp)
{
	bitmap_zero(colormask_bits(dstp), NR_COLORS);
}

static inline void colormask_copy(struct colormask* dstp, 
				const struct colormask *srcp)
{
	bitmap_copy(colormask_bits(dstp), colormask_bits(srcp), NR_COLORS);
}

static inline bool colormask_empty(const struct colormask *srcp)
{
	return bitmap_empty(colormask_bits(srcp), NR_COLORS);
}

static inline unsigned int colormask_first(const struct colormask *srcp)
{
	return find_first_bit(colormask_bits(srcp), NR_COLORS);
}

static inline unsigned int colormask_next(int n, const struct colormask *srcp)
{
	return find_next_bit(colormask_bits(srcp), NR_COLORS, n+1);
}

static inline unsigned int colormask_weight(const struct colormask *srcp)
{
	return bitmap_weight(colormask_bits(srcp), NR_COLORS);
}

int set_colors_allowed_ptr(struct task_struct *p, const struct colormask *new_mask);

#endif
