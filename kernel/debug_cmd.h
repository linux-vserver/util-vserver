#ifndef _VX_DEBUG_CMD_H
#define _VX_DEBUG_CMD_H


/* debug commands */

#define VCMD_dump_history	VC_CMD(DEBUG, 1, 0)

#define VCMD_read_history	VC_CMD(DEBUG, 5, 0)
#define VCMD_read_monitor	VC_CMD(DEBUG, 6, 0)

struct  vcmd_read_history_v0 {
	uint32_t index;
	uint32_t count;
	char *data;
};

struct  vcmd_read_monitor_v0 {
	uint32_t index;
	uint32_t count;
	char *data;
};


#endif	/* _VX_DEBUG_CMD_H */
