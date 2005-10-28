#ifndef _VX_SCHED_DEF_H
#define _VX_SCHED_DEF_H

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
	int vavavoom;			/* last calculated vavavoom */

	cpumask_t cpus_allowed;		/* cpu mask for context */

	struct _vx_ticks cpu[NR_CPUS];
};


#ifdef CONFIG_VSERVER_DEBUG

static inline void __dump_vx_sched(struct _vx_sched *sched)
{
	printk("\t_vx_sched:\n");
	printk("\t tokens: %4d, %4d, %4d, %4d, %4d\n",
		atomic_read(&sched->tokens),
		sched->fill_rate, sched->interval,
		sched->tokens_min, sched->tokens_max);
	printk("\t priority = %4d, %4d\n",
		sched->priority_bias, sched->vavavoom);
}

#endif

#endif	/* _VX_SCHED_DEF_H */
