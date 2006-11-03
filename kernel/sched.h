#ifndef _VX_SCHED_H
#define _VX_SCHED_H


#ifdef	__KERNEL__

struct timespec;

void vx_vsi_uptime(struct timespec *, struct timespec *);


struct vx_info;

void vx_update_load(struct vx_info *);


int vx_tokens_recalc(struct _vx_sched_pc *,
	unsigned long *, unsigned long *, int [2]);

void vx_update_sched_param(struct _vx_sched *sched,
	struct _vx_sched_pc *sched_pc);

#endif	/* __KERNEL__ */
#else	/* _VX_SCHED_H */
#warning duplicate inclusion
#endif	/* _VX_SCHED_H */
