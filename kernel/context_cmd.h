#ifndef _VX_CONTEXT_CMD_H
#define _VX_CONTEXT_CMD_H


/* vinfo commands */

#define VCMD_task_xid		VC_CMD(VINFO, 1, 0)


#define VCMD_vx_info		VC_CMD(VINFO, 5, 0)

struct	vcmd_vx_info_v0 {
	uint32_t xid;
	uint32_t initpid;
	/* more to come */
};


#define VCMD_ctx_stat		VC_CMD(VSTAT, 0, 0)

struct	vcmd_ctx_stat_v0 {
	uint32_t usecnt;
	uint32_t tasks;
	/* more to come */
};


/* context commands */

#define VCMD_ctx_create_v0	VC_CMD(VPROC, 1, 0)
#define VCMD_ctx_create		VC_CMD(VPROC, 1, 1)

struct	vcmd_ctx_create {
	uint64_t flagword;
};

#define VCMD_ctx_migrate_v0	VC_CMD(PROCMIG, 1, 0)
#define VCMD_ctx_migrate	VC_CMD(PROCMIG, 1, 1)

struct	vcmd_ctx_migrate {
	uint64_t flagword;
};



/* flag commands */

#define VCMD_get_cflags		VC_CMD(FLAGS, 1, 0)
#define VCMD_set_cflags		VC_CMD(FLAGS, 2, 0)

struct	vcmd_ctx_flags_v0 {
	uint64_t flagword;
	uint64_t mask;
};



/* context caps commands */

#define VCMD_get_ccaps_v0	VC_CMD(FLAGS, 3, 0)
#define VCMD_set_ccaps_v0	VC_CMD(FLAGS, 4, 0)

struct	vcmd_ctx_caps_v0 {
	uint64_t bcaps;
	uint64_t ccaps;
	uint64_t cmask;
};

#define VCMD_get_ccaps		VC_CMD(FLAGS, 3, 1)
#define VCMD_set_ccaps		VC_CMD(FLAGS, 4, 1)

struct	vcmd_ctx_caps_v1 {
	uint64_t ccaps;
	uint64_t cmask;
};



/* bcaps commands */

#define VCMD_get_bcaps		VC_CMD(FLAGS, 9, 0)
#define VCMD_set_bcaps		VC_CMD(FLAGS,10, 0)

struct	vcmd_bcaps {
	uint64_t bcaps;
	uint64_t bmask;
};

#endif	/* _VX_CONTEXT_CMD_H */
