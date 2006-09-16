#ifndef _VX_SIGNAL_H
#define _VX_SIGNAL_H


#ifdef	__KERNEL__

struct vx_info;

int vx_info_kill(struct vx_info *, int, int);

#endif	/* __KERNEL__ */
#else	/* _VX_SIGNAL_H */
#warning duplicate inclusion
#endif	/* _VX_SIGNAL_H */
