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
	int32_t reboot_cmd;
	int32_t exit_code;
};

#ifdef	__KERNEL__

extern int vc_ctx_kill(struct vx_info *, void __user *);
extern int vc_wait_exit(struct vx_info *, void __user *);

#endif	/* __KERNEL__ */

/*  process alteration commands */

#define VCMD_get_pflags		VC_CMD(PROCALT, 1, 0)
#define VCMD_set_pflags		VC_CMD(PROCALT, 2, 0)

struct	vcmd_pflags_v0 {
	uint32_t flagword;
	uint32_t mask;
};

#ifdef	__KERNEL__

extern int vc_get_pflags(uint32_t pid, void __user *);
extern int vc_set_pflags(uint32_t pid, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_SIGNAL_CMD_H */
