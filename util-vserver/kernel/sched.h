#if	defined(__KERNEL__) && defined(_VX_INFO_DEF_)

#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <asm/param.h>
#include <asm/cpumask.h>

/* context sub struct */

struct _vx_sched {
	spinlock_t tokens_lock; /* lock for this structure */

	int fill_rate;		/* Fill rate: add X tokens... */
	int interval;		/* Divisor:   per Y jiffies   */
	int tokens;		/* number of CPU tokens in this context */
	int tokens_min;		/* Limit:     minimum for unhold */
	int tokens_max;		/* Limit:     no more than N tokens */
	uint32_t jiffies;	/* add an integral multiple of Y to this */

	uint64_t ticks;		/* token tick events */
	cpumask_t cpus_allowed;	/* cpu mask for context */
};

static inline void vx_info_init_sched(struct _vx_sched *sched)
{
        /* scheduling; hard code starting values as constants */
        sched->fill_rate	= 1;
        sched->interval	= 4;
        sched->tokens	= HZ >> 2;
        sched->tokens_min	= HZ >> 4;
        sched->tokens_max	= HZ >> 1;
        sched->jiffies		= jiffies;
        sched->tokens_lock	= SPIN_LOCK_UNLOCKED;
}

static inline int vx_info_proc_sched(struct _vx_sched *sched, char *buffer)
{
	return sprintf(buffer,
		"Ticks:\t%16lld\n"
		"Token:\t\t%8d\n"
		"FillRate:\t%8d\n"
		"Interval:\t%8d\n"		
		"TokensMin:\t%8d\n"
		"TokensMax:\t%8d\n"
		,sched->ticks
		,sched->tokens
		,sched->fill_rate
		,sched->interval
		,sched->tokens_min
		,sched->tokens_max
		);
}


#else	/* _VX_INFO_DEF_ */
#ifndef _VX_SCHED_H
#define _VX_SCHED_H

#include "switch.h"

/*  sched vserver commands */

#define VCMD_set_sched_v1	VC_CMD(SYSTEST, 1, 1)

struct  vcmd_set_sched_v1 {
	int32_t fill_rate;
	int32_t period;
	int32_t fill_level;
	int32_t bucket_size;
};

#define VCMD_set_sched		VC_CMD(SCHED, 1, 2)

struct  vcmd_set_sched_v2 {
	int32_t fill_rate;
	int32_t interval;
	int32_t tokens;
	int32_t tokens_min;
	int32_t tokens_max;
	uint64_t cpu_mask;
};

#define SCHED_KEEP		(-2)

#ifdef	__KERNEL__

extern int vc_set_sched_v1(uint32_t, void *);
extern int vc_set_sched(uint32_t, void *);


#define VAVAVOOM_RATIO		50

#include "context.h"


/* scheduling stuff */

int effective_vavavoom(struct task_struct *, int);

int vx_tokens_recalc(struct vx_info *);

/* update the token allocation for a process */
static inline int vx_tokens_avail(struct task_struct *tsk)
{
	struct vx_info *vxi = tsk->vx_info;
	int tokens;

	spin_lock(&vxi->sched.tokens_lock);
	tokens = vx_tokens_recalc(vxi);
	spin_unlock(&vxi->sched.tokens_lock);
	return tokens;
}

/* new stuff ;) */

static inline int vx_need_resched(struct task_struct *p, struct vx_info *vxi)
{
	p->time_slice--;
	if (vxi) {
		int tokens = 0;

		if (vxi->sched.tokens > 0) {
			spin_lock(&vxi->sched.tokens_lock);
			tokens = --vxi->sched.tokens;
			spin_unlock(&vxi->sched.tokens_lock);
		}
		return ((p->time_slice == 0) || (tokens == 0));
	} else
		return (p->time_slice == 0);
}


#endif	/* __KERNEL__ */

#endif	/* _VX_SCHED_H */
#endif
