#ifndef _VX_LIMIT_DEF_H
#define _VX_LIMIT_DEF_H

#include <asm/atomic.h>
#include <asm/resource.h>

#include "limit.h"


struct _vx_res_limit {
	rlim_t soft;		/* Context soft limit */
	rlim_t hard;		/* Context hard limit */

	rlim_atomic_t rcur;	/* Current value */
	rlim_t rmin;		/* Context minimum */
	rlim_t rmax;		/* Context maximum */

	atomic_t lhit;		/* Limit hits */
};

/* context sub struct */

struct _vx_limit {
	struct _vx_res_limit res[NUM_LIMITS];
};

#ifdef CONFIG_VSERVER_DEBUG

static inline void __dump_vx_limit(struct _vx_limit *limit)
{
	int i;

	printk("\t_vx_limit:");
	for (i=0; i<NUM_LIMITS; i++) {
		printk("\t [%2d] = %8lu %8lu/%8lu, %8ld/%8ld, %8d\n",
			i, (unsigned long)__rlim_get(limit, i),
			(unsigned long)__rlim_rmin(limit, i),
			(unsigned long)__rlim_rmax(limit, i),
			(long)__rlim_soft(limit, i),
			(long)__rlim_hard(limit, i),
			atomic_read(&__rlim_lhit(limit, i)));
	}
}

#endif

#endif	/* _VX_LIMIT_DEF_H */
