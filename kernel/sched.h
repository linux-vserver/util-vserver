#ifndef _VX_SCHED_H
#define _VX_SCHED_H

#ifdef	__KERNEL__

struct timespec;

void vx_vsi_uptime(struct timespec *, struct timespec *);


struct vx_info;

void vx_update_load(struct vx_info *);


struct task_struct;

int vx_effective_vavavoom(struct vx_info *, int);

int vx_tokens_recalc(struct vx_info *);

#endif	/* __KERNEL__ */
#else	/* _VX_SCHED_H */
#warning duplicate inclusion
#endif	/* _VX_SCHED_H */
