#ifndef _VX_DLIMIT_CMD_H
#define _VX_DLIMIT_CMD_H


/*  dlimit vserver commands */

#define VCMD_add_dlimit		VC_CMD(DLIMIT, 1, 0)
#define VCMD_rem_dlimit		VC_CMD(DLIMIT, 2, 0)

#define VCMD_set_dlimit		VC_CMD(DLIMIT, 5, 0)
#define VCMD_get_dlimit		VC_CMD(DLIMIT, 6, 0)

struct	vcmd_ctx_dlimit_base_v0 {
	const char *name;
	uint32_t flags;
};

struct	vcmd_ctx_dlimit_v0 {
	const char *name;
	uint32_t space_used;			/* used space in kbytes */
	uint32_t space_total;			/* maximum space in kbytes */
	uint32_t inodes_used;			/* used inodes */
	uint32_t inodes_total;			/* maximum inodes */
	uint32_t reserved;			/* reserved for root in % */
	uint32_t flags;
};

#define CDLIM_UNSET		((uint32_t)0UL)
#define CDLIM_INFINITY		((uint32_t)~0UL)
#define CDLIM_KEEP		((uint32_t)~1UL)

#endif	/* _VX_DLIMIT_CMD_H */
