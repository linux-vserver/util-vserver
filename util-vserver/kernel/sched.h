/* _VX_SCHED_H defined below */

#if	defined(__KERNEL__) && defined(_VX_INFO_DEF_)

#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/cpumask.h>
#include <asm/atomic.h>
#include <asm/param.h>

struct _vx_ticks {
	uint64_t user_ticks;		/* token tick events */
	uint64_t sys_ticks;		/* token tick events */
	uint64_t hold_ticks;		/* token ticks paused */
	uint64_t unused[5];		/* cacheline ? */
};

/* context sub struct */

struct _vx_sched {
	atomic_t tokens;		/* number of CPU tokens */
	spinlock_t tokens_lock;		/* lock for token bucket */

	int fill_rate;			/* Fill rate: add X tokens... */
	int interval;			/* Divisor:   per Y jiffies   */
	int tokens_min;			/* Limit:     minimum for unhold */
	int tokens_max;			/* Limit:     no more than N tokens */
	uint32_t jiffies;		/* last time accounted */

	int priority_bias;		/* bias offset for priority */
	cpumask_t cpus_allowed;		/* cpu mask for context */

	struct _vx_ticks cpu[NR_CPUS];
};

static inline void vx_info_init_sched(struct _vx_sched *sched)
{
	int i;

	/* scheduling; hard code starting values as constants */
	sched->fill_rate	= 1;
	sched->interval		= 4;
	sched->tokens_min	= HZ >> 4;
	sched->tokens_max	= HZ >> 1;
	sched->jiffies		= jiffies;
	sched->tokens_lock	= SPIN_LOCK_UNLOCKED;

	atomic_set(&sched->tokens, HZ >> 2);
	sched->cpus_allowed	= CPU_MASK_ALL;
	sched->priority_bias	= 0;

	for_each_cpu(i) {
		sched->cpu[i].user_ticks	= 0;
		sched->cpu[i].sys_ticks		= 0;
		sched->cpu[i].hold_ticks	= 0;
	}
}

static inline void vx_info_exit_sched(struct _vx_sched *sched)
{
	return;
}

static inline int vx_info_proc_sched(struct _vx_sched *sched, char *buffer)
{
	int length = 0;
	int i;

	length += sprintf(buffer,
		"Token:\t\t%8d\n"
		"FillRate:\t%8d\n"
		"Interval:\t%8d\n"
		"TokensMin:\t%8d\n"
		"TokensMax:\t%8d\n"
		"PrioBias:\t%8d\n"
		,atomic_read(&sched->tokens)
		,sched->fill_rate
		,sched->interval
		,sched->tokens_min
		,sched->tokens_max
		,sched->priority_bias
		);

	for_each_online_cpu(i) {
		length += sprintf(buffer + length,
			"cpu %d: %lld %lld %lld\n"
			,i
			,sched->cpu[i].user_ticks
			,sched->cpu[i].sys_ticks
			,sched->cpu[i].hold_ticks
			);
	}

	return length;
}


#else	/* _VX_INFO_DEF_ */
#ifndef _VX_SCHED_H
#define _VX_SCHED_H

#include "switch.h"

/*  sched vserver commands */

#define VCMD_set_sched_v2	VC_CMD(SCHED, 1, 2)
#define VCMD_set_sched		VC_CMD(SCHED, 1, 3)

struct	vcmd_set_sched_v2 {
	int32_t fill_rate;
	int32_t interval;
	int32_t tokens;
	int32_t tokens_min;
	int32_t tokens_max;
	uint64_t cpu_mask;
};

struct	vcmd_set_sched_v3 {
	uint32_t set_mask;
	int32_t fill_rate;
	int32_t interval;
	int32_t tokens;
	int32_t tokens_min;
	int32_t tokens_max;
	int32_t priority_bias;
};


#define VXSM_FILL_RATE		0x0001
#define VXSM_INTERVAL		0x0002
#define VXSM_TOKENS		0x0010
#define VXSM_TOKENS_MIN		0x0020
#define VXSM_TOKENS_MAX		0x0040
#define VXSM_PRIO_BIAS		0x0100

#define SCHED_KEEP		(-2)

#ifdef	__KERNEL__

extern int vc_set_sched_v1(uint32_t, void __user *);
extern int vc_set_sched_v2(uint32_t, void __user *);
extern int vc_set_sched(uint32_t, void __user *);


#define VAVAVOOM_RATIO		50

#define MAX_PRIO_BIAS		20
#define MIN_PRIO_BIAS		-20

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

static inline int vx_need_resched(struct task_struct *p)
{
#ifdef	CONFIG_VSERVER_HARDCPU
	struct vx_info *vxi = p->vx_info;
#endif
	int slice = --p->time_slice;

#ifdef	CONFIG_VSERVER_HARDCPU
	if (vxi) {
		int tokens;

		if ((tokens = vx_tokens_avail(vxi)) > 0)
			vx_consume_token(vxi);
		/* for tokens > 0, one token was consumed */
		if (tokens < 2)
			return 1;
	}
#endif
	return (slice == 0);
}


static inline void vx_onhold_inc(struct vx_info *vxi)
{
	int onhold = atomic_read(&vxi->cvirt.nr_onhold);

	atomic_inc(&vxi->cvirt.nr_onhold);
	if (!onhold)
		vxi->cvirt.onhold_last = jiffies;
}

static inline void __vx_onhold_update(struct vx_info *vxi)
{
	int cpu = smp_processor_id();
	uint32_t now = jiffies;
	uint32_t delta = now - vxi->cvirt.onhold_last;

	vxi->cvirt.onhold_last = now;
	vxi->sched.cpu[cpu].hold_ticks += delta;
}

static inline void vx_onhold_dec(struct vx_info *vxi)
{
	if (atomic_dec_and_test(&vxi->cvirt.nr_onhold))
		__vx_onhold_update(vxi);
}

#endif	/* __KERNEL__ */

#endif	/* _VX_SCHED_H */
#endif
