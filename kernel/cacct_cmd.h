#ifndef _VX_CACCT_CMD_H
#define _VX_CACCT_CMD_H


/* virtual host info name commands */

#define VCMD_sock_stat		VC_CMD(VSTAT, 5, 0)

struct	vcmd_sock_stat_v0 {
	uint32_t field;
	uint32_t count[3];
	uint64_t total[3];
};


#endif	/* _VX_CACCT_CMD_H */
