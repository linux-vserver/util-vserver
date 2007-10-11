#ifndef _VX_DEVICE_CMD_H
#define _VX_DEVICE_CMD_H


/*  device vserver commands */

#define VCMD_set_mapping	VC_CMD(DEVICE, 1, 0)
#define VCMD_unset_mapping	VC_CMD(DEVICE, 2, 0)

struct	vcmd_set_mapping_v0 {
	const char *device;
	const char *target;
	uint32_t flags;
};


#endif	/* _VX_DEVICE_CMD_H */
