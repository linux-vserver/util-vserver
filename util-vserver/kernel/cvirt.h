/* _VX_CVIRT_H defined below */

#if	defined(__KERNEL__) && defined(_VX_INFO_DEF_)

#include <linux/utsname.h>
#include <linux/rwsem.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <asm/atomic.h>

/* context sub struct */

struct _vx_cvirt {
	int max_threads;		/* maximum allowed threads */
	atomic_t nr_threads;		/* number of current threads */
	atomic_t nr_running;		/* number of running threads */

	atomic_t nr_onhold;		/* processes on hold */
	uint32_t onhold_last;		/* jiffies when put on hold */

	struct timespec bias_idle;
	uint64_t bias_jiffies;		/* context creation point */

	struct new_utsname utsname;

	spinlock_t load_lock;		/* lock for the load averages */
	uint32_t load_last;		/* last time load was cacled */
	uint32_t load[3];		/* load averages 1,5,15 */

	struct cpu_usage_stat cpustat[NR_CPUS];
};

struct sock_acc {
	atomic_t count;
	atomic_t total;
};

struct _vx_cacct {
	unsigned long total_forks;

	struct sock_acc sock[5][3];
};


static inline long vx_sock_count(struct _vx_cacct *cacct, int type, int pos)
{
	return atomic_read(&cacct->sock[type][pos].count);
}


static inline long vx_sock_total(struct _vx_cacct *cacct, int type, int pos)
{
	return atomic_read(&cacct->sock[type][pos].total);
}


extern uint64_t vx_idle_jiffies(void);

static inline void vx_info_init_cvirt(struct _vx_cvirt *cvirt)
{
	uint64_t idle_jiffies = vx_idle_jiffies();

	cvirt->bias_jiffies = get_jiffies_64();
	jiffies_to_timespec(idle_jiffies, &cvirt->bias_idle);
	atomic_set(&cvirt->nr_threads, 0);
	atomic_set(&cvirt->nr_running, 0);
	atomic_set(&cvirt->nr_onhold, 0);

	down_read(&uts_sem);
	cvirt->utsname = system_utsname;
	up_read(&uts_sem);

	spin_lock_init(&cvirt->load_lock);
	cvirt->load_last = jiffies;
	cvirt->load[0] = 0;
	cvirt->load[1] = 0;
	cvirt->load[2] = 0;
}

static inline void vx_info_exit_cvirt(struct _vx_cvirt *cvirt)
{
#ifdef	CONFIG_VSERVER_DEBUG
	int value;

	if ((value = atomic_read(&cvirt->nr_threads)))
		printk("!!! cvirt: %p[nr_threads] = %d on exit.\n",
			cvirt, value);
	if ((value = atomic_read(&cvirt->nr_running)))
		printk("!!! cvirt: %p[nr_running] = %d on exit.\n",
			cvirt, value);
#endif
	return;
}

static inline void vx_info_init_cacct(struct _vx_cacct *cacct)
{
	int i,j;

	for (i=0; i<5; i++) {
		for (j=0; j<3; j++) {
			atomic_set(&cacct->sock[i][j].count, 0);
			atomic_set(&cacct->sock[i][j].total, 0);
		}
	}
}

static inline void vx_info_exit_cacct(struct _vx_cacct *cacct)
{
	return;
}

#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)


static inline int vx_info_proc_cvirt(struct _vx_cvirt *cvirt, char *buffer)
{
	int length = 0;
	int a, b, c;

	length += sprintf(buffer + length,
		"BiasJiffies:\t%lld\n", (long long int)cvirt->bias_jiffies);
	length += sprintf(buffer + length,
		"SysName:\t%.*s\n"
		"NodeName:\t%.*s\n"
		"Release:\t%.*s\n"
		"Version:\t%.*s\n"
		"Machine:\t%.*s\n"
		"DomainName:\t%.*s\n"
		,__NEW_UTS_LEN, cvirt->utsname.sysname
		,__NEW_UTS_LEN, cvirt->utsname.nodename
		,__NEW_UTS_LEN, cvirt->utsname.release
		,__NEW_UTS_LEN, cvirt->utsname.version
		,__NEW_UTS_LEN, cvirt->utsname.machine
		,__NEW_UTS_LEN, cvirt->utsname.domainname
		);

	a = cvirt->load[0] + (FIXED_1/200);
	b = cvirt->load[1] + (FIXED_1/200);
	c = cvirt->load[2] + (FIXED_1/200);
	length += sprintf(buffer + length,
		"nr_threads:\t%d\n"
		"nr_running:\t%d\n"
		"nr_onhold:\t%d\n"
		"loadavg:\t%d.%02d %d.%02d %d.%02d\n"
		,atomic_read(&cvirt->nr_threads)
		,atomic_read(&cvirt->nr_running)
		,atomic_read(&cvirt->nr_onhold)
		,LOAD_INT(a), LOAD_FRAC(a)
		,LOAD_INT(b), LOAD_FRAC(b)
		,LOAD_INT(c), LOAD_FRAC(c)
		);
	return length;
}

static inline int vx_info_proc_cacct(struct _vx_cacct *cacct, char *buffer)
{
	int i,j, length = 0;
	static char *type[] = { "UNSPEC", "UNIX", "INET", "INET6", "OTHER" };

	for (i=0; i<5; i++) {
		length += sprintf(buffer + length,
			"%s:", type[i]);
		for (j=0; j<3; j++) {
			length += sprintf(buffer + length,
				"\t%12lu/%-12lu"
				,vx_sock_count(cacct, i, j)
				,vx_sock_total(cacct, i, j)
				);
		}
		buffer[length++] = '\n';
	}
	length += sprintf(buffer + length,
		"forks:\t%lu\n", cacct->total_forks);
	return length;
}

#else	/* _VX_INFO_DEF_ */
#ifndef _VX_CVIRT_H
#define _VX_CVIRT_H

#include "switch.h"

/*  cvirt vserver commands */


#ifdef	__KERNEL__

struct timespec;

void vx_vsi_uptime(struct timespec *, struct timespec *);

struct vx_info;

void vx_update_load(struct vx_info *);


#endif	/* __KERNEL__ */

#endif	/* _VX_CVIRT_H */
#endif
