#ifndef _VX_LIMIT_H
#define _VX_LIMIT_H


#define VLIMIT_NSOCK	16
#define VLIMIT_OPENFD	17
#define VLIMIT_ANON	18
#define VLIMIT_SHMEM	19

#ifdef	__KERNEL__

struct sysinfo;

void vx_vsi_meminfo(struct sysinfo *);
void vx_vsi_swapinfo(struct sysinfo *);

#define NUM_LIMITS	24

#endif	/* __KERNEL__ */
#endif	/* _VX_LIMIT_H */
