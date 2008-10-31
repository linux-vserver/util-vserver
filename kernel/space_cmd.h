#ifndef _VX_SPACE_CMD_H
#define _VX_SPACE_CMD_H


#define VCMD_enter_space_v0	VC_CMD(PROCALT, 1, 0)
#define VCMD_enter_space_v1	VC_CMD(PROCALT, 1, 1)
#define VCMD_enter_space	VC_CMD(PROCALT, 1, 2)

/* XXX: This is not available in recent kernels */
#define VCMD_cleanup_namespace	VC_CMD(PROCALT, 2, 0)

#define VCMD_set_space_v0	VC_CMD(PROCALT, 3, 0)
#define VCMD_set_space_v1	VC_CMD(PROCALT, 3, 1)
#define VCMD_set_space		VC_CMD(PROCALT, 3, 2)

#define VCMD_get_space_mask_v0	VC_CMD(PROCALT, 4, 0)

#define VCMD_get_space_mask	VC_CMD(VSPACE, 0, 1)
#define VCMD_get_space_default	VC_CMD(VSPACE, 1, 0)


struct	vcmd_space_mask_v1 {
	uint64_t mask;
};

struct	vcmd_space_mask_v2 {
	uint64_t mask;
	uint32_t index;
};


#endif	/* _VX_SPACE_CMD_H */
