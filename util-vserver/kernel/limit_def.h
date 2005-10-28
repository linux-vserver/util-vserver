#ifndef _VX_LIMIT_DEF_H
#define _VX_LIMIT_DEF_H

#include <asm/atomic.h>
#include <asm/resource.h>

#include "limit.h"

/* context sub struct */

struct _vx_limit {
//	atomic_t ticks;

	unsigned long rlim[NUM_LIMITS];		/* Context limit */
	unsigned long rmax[NUM_LIMITS];		/* Context maximum */
	atomic_t rcur[NUM_LIMITS];		/* Current value */
	atomic_t lhit[NUM_LIMITS];		/* Limit hits */
};

#ifdef CONFIG_VSERVER_DEBUG

static inline void __dump_vx_limit(struct _vx_limit *limit)
{
	int i;

	printk("\t_vx_limit:");
	for (i=0; i<NUM_LIMITS; i++) {
		printk("\t [%2d] = %8lu, %8lu, %8d, %8d\n",
			i, limit->rlim[i], limit->rmax[i],
			atomic_read(&limit->rcur[i]),
			atomic_read(&limit->lhit[i]));
	}
}

#endif

#endif	/* _VX_LIMIT_DEF_H */
