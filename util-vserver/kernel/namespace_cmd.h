#ifndef _VX_NAMESPACE_CMD_H
#define _VX_NAMESPACE_CMD_H

#define VCMD_enter_namespace	VC_CMD(PROCALT, 1, 0)
#define VCMD_cleanup_namespace	VC_CMD(PROCALT, 2, 0)
#define VCMD_set_namespace	VC_CMD(PROCALT, 3, 0)


#ifdef	__KERNEL__

extern int vc_enter_namespace(uint32_t, void __user *);
extern int vc_cleanup_namespace(uint32_t, void __user *);
extern int vc_set_namespace(uint32_t, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_NAMESPACE_CMD_H */
