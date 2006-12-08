#ifndef _VX_SPACE_CMD_H
#define _VX_SPACE_CMD_H


#define VCMD_enter_space_v0	VC_CMD(PROCALT, 1, 0)
#define VCMD_enter_space	VC_CMD(PROCALT, 1, 1)

/* XXX: This is not available in recent kernels */
#define VCMD_cleanup_namespace	VC_CMD(PROCALT, 2, 0)

#define VCMD_set_space_v0	VC_CMD(PROCALT, 3, 0)
#define VCMD_set_space		VC_CMD(PROCALT, 3, 1)

#define VCMD_get_space_mask	VC_CMD(PROCALT, 4, 0)


struct	vcmd_space_mask {
	uint64_t mask;
};


#endif	/* _VX_SPACE_CMD_H */
