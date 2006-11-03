#ifndef _VX_NAMESPACE_CMD_H
#define _VX_NAMESPACE_CMD_H


#define VCMD_enter_namespace	VC_CMD(PROCALT, 1, 0)
/* XXX: This is not available in recent kernels */
#define VCMD_cleanup_namespace	VC_CMD(PROCALT, 2, 0)
#define VCMD_set_namespace_v0	VC_CMD(PROCALT, 3, 0)
#define VCMD_set_namespace	VC_CMD(PROCALT, 3, 1)


#ifdef	__KERNEL__

extern int vc_enter_namespace(struct vx_info *, void __user *);
extern int vc_set_namespace(struct vx_info *, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_NAMESPACE_CMD_H */
