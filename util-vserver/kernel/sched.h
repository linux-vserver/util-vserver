#if	defined(__KERNEL__) && defined(_VX_INFO_DEF_)

#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <asm/atomic.h>
#include <asm/param.h>
#include <asm/cpumask.h>

/* context sub struct */

struct _vx_sched {
	spinlock_t tokens_lock; /* lock for this structure */

	int fill_rate;		/* Fill rate: add X tokens... */
	int interval;		/* Divisor:   per Y jiffies   */
	atomic_t tokens;	/* number of CPU tokens in this context */
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
        sched->interval		= 4;
        sched->tokens_min	= HZ >> 4;
        sched->tokens_max	= HZ >> 1;
        sched->jiffies		= jiffies;
        sched->tokens_lock	= SPIN_LOCK_UNLOCKED;

        atomic_set(&sched->tokens, HZ >> 2);
	sched->cpus_allowed	= CPU_MASK_ALL;
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
		,atomic_read(&sched->tokens)
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

extern int vc_set_sched_v1(uint32_t, void __user *);
extern int vc_set_sched(uint32_t, void __user *);


#define VAVAVOOM_RATIO		50

#include "context.h"


/* scheduling stuff */

int effective_vavavoom(struct task_struct *, int);

int vx_tokens_recalc(struct vx_info *);

/* new stuff ;) */

static inline int vx_tokens_avail(struct vx_info *vxi)
{
	return atomic_read(&vxi->sched.tokens);
}

static inline void vx_consume_token(struct vx_info *vxi)
{
	atomic_dec(&vxi->sched.tokens);
}

static inline int vx_need_resched(struct task_struct *p, struct vx_info *vxi)
{
	p->time_slice--;
	if (vxi) {
		int tokens;
		if ((tokens = vx_tokens_avail(vxi)) > 0)
			vx_consume_token(vxi);

		return ((p->time_slice == 0) || (tokens < 1));
	} else
		return (p->time_slice == 0);
}


#endif	/* __KERNEL__ */

#endif	/* _VX_SCHED_H */
#endif
