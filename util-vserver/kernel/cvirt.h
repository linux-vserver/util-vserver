#if	defined(__KERNEL__) && defined(_VX_INFO_DEF_)

#include <linux/utsname.h>
#include <linux/rwsem.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <asm/atomic.h>

/* context sub struct */

struct sock_acc {
	atomic_t count;
	atomic_t total;
};

struct _vx_cvirt {
	int nr_threads;
	int nr_running;
	int max_threads;
	unsigned long total_forks;

	unsigned int bias_cswtch;
	struct timespec bias_idle;
	struct timespec bias_tp;
	uint64_t bias_jiffies;

	struct new_utsname utsname;

	struct sock_acc sock[5][3];
};


static inline long vx_sock_count(struct _vx_cvirt *cvirt, int type, int pos)
{
	return atomic_read(&cvirt->sock[type][pos].count);
}


static inline long vx_sock_total(struct _vx_cvirt *cvirt, int type, int pos)
{
	return atomic_read(&cvirt->sock[type][pos].total);
}


extern uint64_t vx_idle_jiffies(void);

static inline void vx_info_init_cvirt(struct _vx_cvirt *cvirt)
{
	int i,j;
	uint64_t idle_jiffies = vx_idle_jiffies();

	cvirt->nr_threads = 1;
	// new->virt.bias_cswtch = kstat.context_swtch;
	cvirt->bias_jiffies = get_jiffies_64();

	jiffies_to_timespec(idle_jiffies, &cvirt->bias_idle);
	do_posix_clock_monotonic_gettime(&cvirt->bias_tp);

	down_read(&uts_sem);
	cvirt->utsname = system_utsname;
	up_read(&uts_sem);

	for (i=0; i<5; i++) {
		for (j=0; j<3; j++) {
			atomic_set(&cvirt->sock[i][j].count, 0);
			atomic_set(&cvirt->sock[i][j].total, 0);
		}
	}
}

static inline int vx_info_proc_cvirt(struct _vx_cvirt *cvirt, char *buffer)
{
	int i,j, length = 0;
	static char *type[] = { "UNSPEC", "UNIX", "INET", "INET6", "OTHER" };

	for (i=0; i<5; i++) {
		length += sprintf(buffer + length,
			"%s:", type[i]);
		for (j=0; j<3; j++) {
			length += sprintf(buffer + length,
				"\t%12lu/%-12lu"
				,vx_sock_count(cvirt, i, j)
				,vx_sock_total(cvirt, i, j)
				);
		}	
		buffer[length++] = '\n';
	}
	return length;
}

#else	/* _VX_INFO_DEF_ */
#ifndef _VX_CVIRT_H
#define _VX_CVIRT_H

#include "switch.h"

/*  cvirt vserver commands */


#ifdef	__KERNEL__

struct timespec;

void vx_vsi_uptime(struct timespec *uptime, struct timespec *idle);

#endif	/* __KERNEL__ */

#endif	/* _VX_CVIRT_H */
#endif
