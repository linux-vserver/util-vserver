#ifndef _VX_LIMIT_DEF_H
#define _VX_LIMIT_DEF_H

#include <asm/atomic.h>
#include <asm/resource.h>

#include "limit.h"

/* context sub struct */

struct _vx_limit {
	atomic_t ticks;

	unsigned long rlim[NUM_LIMITS];		/* Context limit */
	unsigned long rmax[NUM_LIMITS];		/* Context maximum */
	atomic_t rcur[NUM_LIMITS];		/* Current value */
	atomic_t lhit[NUM_LIMITS];		/* Limit hits */
};


#endif	/* _VX_LIMIT_DEF_H */
