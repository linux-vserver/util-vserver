#ifndef _VX_INODE_H
#define _VX_INODE_H

#include "switch.h"

/*  inode vserver commands */

#define VCMD_get_iattr_v0	VC_CMD(INODE, 1, 0)
#define VCMD_set_iattr_v0	VC_CMD(INODE, 2, 0)

#define VCMD_get_iattr		VC_CMD(INODE, 1, 1)
#define VCMD_set_iattr		VC_CMD(INODE, 2, 1)

struct  vcmd_ctx_iattr_v0 {
	/* device handle in id */
	uint64_t ino;
	uint32_t xid;
	uint32_t flags;
	uint32_t mask;
};

struct  vcmd_ctx_iattr_v1 {
	const char __user *name;
	uint32_t xid;
	uint32_t flags;
	uint32_t mask;
};


#define IATTR_XID	0x01000000

#define IATTR_ADMIN	0x00000001
#define IATTR_WATCH	0x00000002
#define IATTR_HIDE	0x00000004
#define IATTR_FLAGS	0x00000007

#define IATTR_BARRIER	0x00010000
#define	IATTR_IUNLINK	0x00020000
#define	IATTR_IMMUTABLE	0x00040000


#ifdef	CONFIG_PROC_SECURE
#define	IATTR_PROC_DEFAULT	( IATTR_ADMIN | IATTR_HIDE )
#define	IATTR_PROC_SYMLINK	( IATTR_ADMIN )
#else
#define	IATTR_PROC_DEFAULT	( IATTR_ADMIN )
#define	IATTR_PROC_SYMLINK	( IATTR_ADMIN )
#endif

#ifdef	__KERNEL__

#define vx_hide_check(c,m)      (((m) & IATTR_HIDE) ? vx_check(c,m) : 1)

extern int vc_get_iattr_v0(uint32_t, void __user *);
extern int vc_set_iattr_v0(uint32_t, void __user *);

extern int vc_get_iattr(uint32_t, void __user *);
extern int vc_set_iattr(uint32_t, void __user *);

#endif	/* __KERNEL__ */

/* inode ioctls */

#define	FIOC_GETXFLG	_IOR('x', 5, long)
#define	FIOC_SETXFLG	_IOW('x', 6, long)

#endif	/* _VX_LEGACY_H */
