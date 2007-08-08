#ifndef _VX_INODE_CMD_H
#define _VX_INODE_CMD_H


/*  inode vserver commands */

#define VCMD_get_iattr		VC_CMD(INODE, 1, 1)
#define VCMD_set_iattr		VC_CMD(INODE, 2, 1)

#define VCMD_fget_iattr		VC_CMD(INODE, 3, 0)
#define VCMD_fset_iattr		VC_CMD(INODE, 4, 0)

struct	vcmd_ctx_iattr_v1 {
	const char *name;
	uint32_t xid;
	uint32_t flags;
	uint32_t mask;
};

struct	vcmd_ctx_fiattr_v0 {
	uint32_t xid;
	uint32_t flags;
	uint32_t mask;
};


#endif	/* _VX_INODE_CMD_H */
