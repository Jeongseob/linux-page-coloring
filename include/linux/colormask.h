#ifndef __LINUX_COLORMASK_H
#define __LINUX_COLORMASK_H

#define NR_COLORS	128

typedef struct colormask { DECLARE_BITMAP(bits, NR_COLORS); } colormask_t;

#endif
