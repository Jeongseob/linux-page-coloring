/*
 * For the most part, I borrowed from Karel Zak (https://github.com/karelzak/util-linux)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>

#include "procutils.h"

#define __NR_set_color	322
#define __NR_get_color	323

#define NCOLORS	128
#define __COLOR_SETSIZE	1024
#define __NCOLORBITS	(8 * sizeof(__color_mask))

#define COLOR_ALLOC_SIZE(count) \
         ((((count) + __NCOLORBITS - 1) / __NCOLORBITS) * sizeof (__color_mask))
#define COLOR_ALLOC(count)   (malloc(COLOR_ALLOC_SIZE(count)))
#define COLOR_FREE(cpuset)   (free(cpuset))
#define colorset_nbits(setsize)   (8 * (setsize))

# define __COLORELT(color)	((color) / __NCOLORBITS)
# define __COLORMASK(color)	((__color_mask) 1 << ((color) % __NCOLORBITS))

# define COLOR_ZERO_S(setsize, colorsetp) \
	do {                                        \
		size_t __i;                                   \
		size_t __imax = (setsize) / sizeof (__color_mask);                  \
		__color_mask *__bits = (colorsetp)->__bits;                   \
		for (__i = 0; __i < __imax; ++__i)                        \
		__bits[__i] = 0;                                \
	} while (0)

# define COLOR_SET_S(color, setsize, colorsetp) \
	({ size_t __color = (color);                           \
	 __color < 8 * (setsize)                           \
	 ? (((__color_mask *) ((colorsetp)->__bits))[__COLORELT (__color)]           \
		 |= __COLORMASK (__color))                            \
	 : 0; })



# define COLOR_ISSET_S(color, setsize, colorsetp) \
	({ size_t __color = (color);                           \
	 __color < 8 * (setsize)                           \
	 ? ((((__color_mask *) ((colorsetp)->__bits))[__COLORELT (__color)]          \
			 & __COLORMASK (__color))) != 0                          \
	 : 0; })


typedef unsigned long int __color_mask;

typedef struct {
	__color_mask __bits[__COLOR_SETSIZE / __NCOLORBITS];
} color_set_t;

struct colorset {
	pid_t		pid;		/* task PID */
	color_set_t	*set;		/* task color mask */
	size_t		setsize;
	char		*buf;		/* buffer for conversion from mask to string */
	size_t		buflen;
	unsigned int	get_only:1;
};

static void usage(FILE* out)
{
	fprintf(out,"Usage: ./colorset [options] [pid | cmd [args...]]\n\n");
	fprintf(out, "Options:\n"
		" -p, operate on existing given pid\n\n");

	exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);

}

static inline int val_to_char(int v)
{
	if (v >= 0 && v < 10)
		return '0' + v;
	else if (v >= 10 && v < 16)
		return ('a' - 10) + v;
	else
		return -1;
}

static inline int char_to_val(int c)
{
	int cl;

	cl = tolower(c);
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (cl >= 'a' && cl <= 'f')
		return cl + (10 - 'a');
	else
		return -1;
}


/*
 * Parses string with COLOR mask.
 */
int colormask_parse(const char *str, color_set_t *set, size_t setsize)
{
	int len = strlen(str);
	const char *ptr = str + len - 1;
	int color = 0;

	/* skip 0x, it's all hex anyway */
	if (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	COLOR_ZERO_S(setsize, set);

	while (ptr >= str) {
		char val;

		/* cpu masks in /sys uses comma as a separator */
		if (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		if (val == (char) -1)
			return -1;
		if (val & 1)
			COLOR_SET_S(color, setsize, set);
		if (val & 2)
			COLOR_SET_S(color + 1, setsize, set);
		if (val & 4)
			COLOR_SET_S(color + 2, setsize, set);
		if (val & 8)
			COLOR_SET_S(color + 3, setsize, set);
		len--;
		ptr--;
		color += 4;
	}

	return 0;
}


/*
 * Returns string with color mask.
 */
char *colormask_create(char *str, size_t len,
		color_set_t *set, size_t setsize)
{
	char *ptr = str;
	char *ret = NULL;
	int color;

	for (color = colorset_nbits(setsize) - 4; color >= 0; color -= 4) {
		char val = 0;

		if (len == (size_t) (ptr - str))
			break;

		if (COLOR_ISSET_S(color, setsize, set))
			val |= 1;
		if (COLOR_ISSET_S(color + 1, setsize, set))
			val |= 2;
		if (COLOR_ISSET_S(color + 2, setsize, set))
			val |= 4;
		if (COLOR_ISSET_S(color + 3, setsize, set))
			val |= 8;

		if (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	return ret ? ret : ptr - 1;
}


static void print_color(struct colorset *cs, int isnew)
{
	char *str, *msg;

	str = colormask_create(cs->buf, cs->buflen, cs->set, cs->setsize);
	msg = isnew ? ("pid %d's new color mask: %s\n") : ("pid %d's current color mask: %s\n");

	printf(msg, cs->pid, str);
}

static void do_colorset(struct colorset *cs, size_t setsize, color_set_t* set)
{
	printf("do_colorset called\n");

	if ( cs->pid ) {
		if ( syscall(__NR_get_color, cs->pid, cs->setsize, cs->set) < 0 ) {
			printf("failed to get pid %d's color affinity", cs->pid);
			return;
		}
		printf("%lx\n", *cs->set->__bits);
		print_color(cs, 0);
	}

	if (cs->get_only) {
		return;
	}

	if ( syscall(__NR_set_color, cs->pid, setsize, set) < 0 ) {
		printf("err\n");
	}

	if ( cs->pid ) {
		if ( syscall(__NR_get_color, cs->pid, cs->setsize, cs->set) < 0 ){
			printf("err\n");	
		}

		print_color(cs, 1);
	}
}

color_set_t* colorset_alloc(int ncolors, size_t *setsize, size_t *nbits)
{
	color_set_t *set = COLOR_ALLOC(ncolors);

	if (!set) {
		return NULL;
	}

	if (setsize) {
		*setsize = COLOR_ALLOC_SIZE(ncolors);	
	}

	if (nbits) {
		*nbits = colorset_nbits(COLOR_ALLOC_SIZE(ncolors));
	}

	return set;
}

int main(int argc, char **argv)
{
	int c;
	color_set_t* new_set;
	size_t new_setsize, nbits;
	pid_t pid = 0;
	struct colorset cs;

	memset(&cs, 0, sizeof(cs));

	while ((c = getopt(argc, argv, "+ph")) != -1) {
		switch (c) {
		case 'p':
			pid = atoi(argv[argc - 1]);
			break;
		case 'h':
			usage(stdout);
			break;
		default:
			usage(stderr);
			break;
		}
	}

	if ((!pid && argc - optind < 2)
	    || (pid && (argc - optind < 1 || argc - optind > 2))) {
		usage(stderr);
	}

	cs.set = colorset_alloc(NCOLORS, &cs.setsize, &nbits);
	if (!cs.set) {
		printf("colorset_alloc failed\n");
		return EXIT_FAILURE;		
	}

	cs.buflen = 7 * nbits;
	cs.buf = malloc(cs.buflen);
	
	new_set = colorset_alloc(NCOLORS, &new_setsize, NULL);
	if (!new_set) {
		printf("colorset_alloc failed\n");
		return EXIT_FAILURE;		
	}

	if (argc - optind == 1) {
		cs.get_only = 1;
	}

	if (colormask_parse(argv[optind], new_set, new_setsize) ) {
		printf("failed to parsec COLOR mask: %s\n", argv[optind]);
		return EXIT_FAILURE;
	}

	if (pid) {
		struct proc_tasks *tasks = proc_open_tasks(pid);
		while (!proc_next_tid(tasks, &cs.pid)) {
			do_colorset(&cs, new_setsize, new_set);
		}
		proc_close_tasks(tasks);

	} else {
		cs.pid = pid;
		do_colorset(&cs, new_setsize, new_set);
	}
	
	free(cs.buf);

	printf("optind: %d\n", optind);
	
	if (!pid) {
		argv += optind + 1;
		printf ("%s\n", argv[0]);
		execvp(argv[0], argv);
	}
	


	return EXIT_SUCCESS;
}
