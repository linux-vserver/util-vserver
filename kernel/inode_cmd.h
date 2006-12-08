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
	const char *name;
	uint32_t xid;
	uint32_t flags;
	uint32_t mask;
};


#endif	/* _VX_INODE_CMD_H */
