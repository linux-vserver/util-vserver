#ifndef _VX_SIGNAL_H
#define _VX_SIGNAL_H

#include "switch.h"

/*  context signalling */

#define VCMD_ctx_kill		VC_CMD(PROCTRL, 1, 0)

struct  vcmd_ctx_kill_v0 {
	int32_t pid;
	int32_t sig;
};

#ifdef	__KERNEL__
extern int vc_ctx_kill(uint32_t, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_SIGNAL_H */
