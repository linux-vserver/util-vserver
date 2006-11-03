#ifndef _VX_SCHED_DEF_H
#define _VX_SCHED_DEF_H

#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/cpumask.h>
#include <asm/atomic.h>
#include <asm/param.h>


/* context sub struct */

struct _vx_sched {
	spinlock_t tokens_lock;		/* lock for token bucket */

	int tokens;			/* number of CPU tokens */
	int fill_rate[2];		/* Fill rate: add X tokens... */
	int interval[2];		/* Divisor:   per Y jiffies   */
	int tokens_min;			/* Limit:     minimum for unhold */
	int tokens_max;			/* Limit:     no more than N tokens */

	unsigned update_mask;		/* which features should be updated */
	cpumask_t update;		/* CPUs which should update */

	int prio_bias;			/* bias offset for priority */
	int vavavoom;			/* last calculated vavavoom */
};

struct _vx_sched_pc {
	int tokens;			/* number of CPU tokens */
	int flags;			/* bucket flags */

	int fill_rate[2];		/* Fill rate: add X tokens... */
	int interval[2];		/* Divisor:   per Y jiffies   */
	int tokens_min;			/* Limit:     minimum for unhold */
	int tokens_max;			/* Limit:     no more than N tokens */

	unsigned long norm_time;	/* last time accounted */
	unsigned long idle_time;	/* non linear time for fair sched */
	unsigned long token_time;	/* token time for accounting */
	unsigned long onhold;		/* jiffies when put on hold */

	uint64_t user_ticks;		/* token tick events */
	uint64_t sys_ticks;		/* token tick events */
	uint64_t hold_ticks;		/* token ticks paused */
};


#define VXSF_ONHOLD	0x0001
#define VXSF_IDLE_TIME	0x0100

#ifdef CONFIG_VSERVER_DEBUG

static inline void __dump_vx_sched(struct _vx_sched *sched)
{
	printk("\t_vx_sched:\n");
	printk("\t tokens: %4d/%4d, %4d/%4d, %4d, %4d\n",
		sched->fill_rate[0], sched->interval[0],
		sched->fill_rate[1], sched->interval[1],
		sched->tokens_min, sched->tokens_max);
	printk("\t priority = %4d, %4d\n",
		sched->prio_bias, sched->vavavoom);
}

#endif

#endif	/* _VX_SCHED_DEF_H */
