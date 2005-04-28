#ifndef _VX_CVIRT_DEF_H
#define _VX_CVIRT_DEF_H

#include <linux/jiffies.h>
#include <linux/utsname.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <asm/atomic.h>


struct _vx_usage_stat {
	uint64_t user;
	uint64_t nice;
	uint64_t system;
	uint64_t softirq;
	uint64_t irq;
	uint64_t idle;
	uint64_t iowait;
};

struct _vx_syslog {
	wait_queue_head_t log_wait;
	spinlock_t logbuf_lock;		/* lock for the log buffer */

	unsigned long log_start; 	/* next char to be read by syslog() */
	unsigned long con_start; 	/* next char to be sent to consoles */
	unsigned long log_end;   	/* most-recently-written-char + 1 */
	unsigned long logged_chars; 	/* #chars since last read+clear operation */

	char log_buf[1024];
};


/* context sub struct */

struct _vx_cvirt {
	int max_threads;		/* maximum allowed threads */
	atomic_t nr_threads;		/* number of current threads */
	atomic_t nr_running;		/* number of running threads */
	atomic_t nr_uninterruptible;	/* number of uninterruptible threads */

	atomic_t nr_onhold;		/* processes on hold */
	uint32_t onhold_last;		/* jiffies when put on hold */

	struct timespec bias_idle;
	struct timespec bias_uptime;	/* context creation point */
	uint64_t bias_clock;		/* offset in clock_t */

	struct new_utsname utsname;

	spinlock_t load_lock;		/* lock for the load averages */
	atomic_t load_updates;		/* nr of load updates done so far */
	uint32_t load_last;		/* last time load was cacled */
	uint32_t load[3];		/* load averages 1,5,15 */

	atomic_t total_forks;		/* number of forks so far */

	struct _vx_usage_stat cpustat[NR_CPUS];

	struct _vx_syslog syslog;
};

struct _vx_sock_acc {
	atomic_t count;
	atomic_t total;
};

/* context sub struct */

struct _vx_cacct {
	unsigned long total_forks;

	struct _vx_sock_acc sock[5][3];
};

#endif	/* _VX_CVIRT_DEF_H */
