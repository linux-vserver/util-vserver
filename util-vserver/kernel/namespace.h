#ifndef _VX_NAMESPACE_H
#define _VX_NAMESPACE_H

#include <linux/types.h>


/* virtual host info names */

#define VCMD_vx_set_vhi_name	VC_CMD(VHOST, 1, 0)
#define VCMD_vx_get_vhi_name	VC_CMD(VHOST, 2, 0)

struct	vcmd_vx_vhi_name_v0 {
	uint32_t field;
	char name[65];
};


enum vx_vhi_name_field {
	VHIN_CONTEXT=0,
	VHIN_SYSNAME,
	VHIN_NODENAME,
	VHIN_RELEASE,
	VHIN_VERSION,
	VHIN_MACHINE,
	VHIN_DOMAINNAME,
};


#ifdef	__KERNEL__

#include <linux/compiler.h>

extern int vc_set_vhi_name(uint32_t, void __user *);
extern int vc_get_vhi_name(uint32_t, void __user *);

#endif	/* __KERNEL__ */

#define VCMD_enter_namespace	VC_CMD(PROCALT, 1, 0)
#define VCMD_cleanup_namespace	VC_CMD(PROCALT, 2, 0)
#define VCMD_set_namespace	VC_CMD(PROCALT, 3, 0)

#ifdef	__KERNEL__

struct vx_info;
struct namespace;
struct fs_struct;
struct vfsmount;

extern int vx_set_namespace(struct vx_info *, struct namespace *, struct fs_struct *);

extern int vc_enter_namespace(uint32_t, void __user *);
extern int vc_cleanup_namespace(uint32_t, void __user *);
extern int vc_set_namespace(uint32_t, void __user *);

#endif	/* __KERNEL__ */
#else	/* _VX_NAMESPACE_H */
#warning duplicate inclusion
#endif	/* _VX_NAMESPACE_H */
