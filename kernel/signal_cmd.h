#ifndef _VX_SIGNAL_CMD_H
#define _VX_SIGNAL_CMD_H


/*  signalling vserver commands */

#define VCMD_ctx_kill		VC_CMD(PROCTRL, 1, 0)
#define VCMD_wait_exit		VC_CMD(EVENT, 99, 0)

struct	vcmd_ctx_kill_v0 {
	int32_t pid;
	int32_t sig;
};

struct	vcmd_wait_exit_v0 {
	int32_t reboot_cmd;
	int32_t exit_code;
};


/*  process alteration commands */

#define VCMD_get_pflags		VC_CMD(PROCALT, 5, 0)
#define VCMD_set_pflags		VC_CMD(PROCALT, 6, 0)

struct	vcmd_pflags_v0 {
	uint32_t flagword;
	uint32_t mask;
};

#endif	/* _VX_SIGNAL_CMD_H */
