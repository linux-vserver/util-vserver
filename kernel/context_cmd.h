#ifndef _VX_CONTEXT_CMD_H
#define _VX_CONTEXT_CMD_H


/* vinfo commands */

#define VCMD_task_xid		VC_CMD(VINFO, 1, 0)

#ifdef	__KERNEL__
extern int vc_task_xid(uint32_t, void __user *);

#endif	/* __KERNEL__ */

#define VCMD_vx_info		VC_CMD(VINFO, 5, 0)

struct	vcmd_vx_info_v0 {
	uint32_t xid;
	uint32_t initpid;
	/* more to come */
};

#ifdef	__KERNEL__
extern int vc_vx_info(uint32_t, void __user *);

#endif	/* __KERNEL__ */


/* context commands */

#define VCMD_ctx_create_v0	VC_CMD(VPROC, 1, 0)
#define VCMD_ctx_create		VC_CMD(VPROC, 1, 1)

struct	vcmd_ctx_create {
	uint64_t flagword;
};

#define VCMD_ctx_migrate	VC_CMD(PROCMIG, 1, 0)

#ifdef	__KERNEL__
extern int vc_ctx_create(uint32_t, void __user *);
extern int vc_ctx_migrate(uint32_t, void __user *);

#endif	/* __KERNEL__ */


/* flag commands */

#define VCMD_get_cflags		VC_CMD(FLAGS, 1, 0)
#define VCMD_set_cflags		VC_CMD(FLAGS, 2, 0)

struct	vcmd_ctx_flags_v0 {
	uint64_t flagword;
	uint64_t mask;
};

#ifdef	__KERNEL__
extern int vc_get_cflags(uint32_t, void __user *);
extern int vc_set_cflags(uint32_t, void __user *);

#endif	/* __KERNEL__ */


/* context caps commands */

#define VCMD_get_ccaps		VC_CMD(FLAGS, 3, 0)
#define VCMD_set_ccaps		VC_CMD(FLAGS, 4, 0)

struct	vcmd_ctx_caps_v0 {
	uint64_t bcaps;
	uint64_t ccaps;
	uint64_t cmask;
};

#ifdef	__KERNEL__
extern int vc_get_ccaps(uint32_t, void __user *);
extern int vc_set_ccaps(uint32_t, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_CONTEXT_CMD_H */
