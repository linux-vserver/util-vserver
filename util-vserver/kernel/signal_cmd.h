#ifndef _VX_SIGNAL_CMD_H
#define _VX_SIGNAL_CMD_H

/*  signalling vserver commands */

#define VCMD_ctx_kill		VC_CMD(PROCTRL, 1, 0)
#define VCMD_wait_exit		VC_CMD(EVENT, 99, 0)

struct	vcmd_ctx_kill_v0 {
	int32_t pid;
	int32_t sig;
};

struct	vcmd_wait_exit_v0 {
	int32_t a;
	int32_t b;
};

#ifdef	__KERNEL__

extern int vc_ctx_kill(uint32_t, void __user *);
extern int vc_wait_exit(uint32_t, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_SIGNAL_CMD_H */
