#ifndef _VX_INODE_CMD_H
#define _VX_INODE_CMD_H


/*  inode vserver commands */

#define VCMD_get_iattr_v0	VC_CMD(INODE, 1, 0)
#define VCMD_set_iattr_v0	VC_CMD(INODE, 2, 0)

#define VCMD_get_iattr		VC_CMD(INODE, 1, 1)
#define VCMD_set_iattr		VC_CMD(INODE, 2, 1)

struct	vcmd_ctx_iattr_v0 {
	/* device handle in id */
	uint64_t ino;
	uint32_t xid;
	uint32_t flags;
	uint32_t mask;
};

struct	vcmd_ctx_iattr_v1 {
	const char __user *name;
	uint32_t xid;
	uint32_t flags;
	uint32_t mask;
};


#ifdef	__KERNEL__


#ifdef	CONFIG_COMPAT

#include <asm/compat.h>

struct	vcmd_ctx_iattr_v1_x32 {
	compat_uptr_t name_ptr;
	uint32_t xid;
	uint32_t flags;
	uint32_t mask;
};

#endif	/* CONFIG_COMPAT */

#include <linux/compiler.h>

extern int vc_get_iattr_v0(uint32_t, void __user *);
extern int vc_set_iattr_v0(uint32_t, void __user *);

extern int vc_get_iattr(uint32_t, void __user *);
extern int vc_set_iattr(uint32_t, void __user *);

#ifdef	CONFIG_COMPAT

extern int vc_get_iattr_x32(uint32_t, void __user *);
extern int vc_set_iattr_x32(uint32_t, void __user *);

#endif	/* CONFIG_COMPAT */

#endif	/* __KERNEL__ */
#endif	/* _VX_INODE_CMD_H */
