#ifndef _VX_NETWORK_CMD_H
#define _VX_NETWORK_CMD_H


/* vinfo commands */

#define VCMD_task_nid		VC_CMD(VINFO, 2, 0)


#define VCMD_nx_info		VC_CMD(VINFO, 6, 0)

struct	vcmd_nx_info_v0 {
	uint32_t nid;
	/* more to come */
};


#define VCMD_net_create_v0	VC_CMD(VNET, 1, 0)
#define VCMD_net_create		VC_CMD(VNET, 1, 1)

struct  vcmd_net_create {
	uint64_t flagword;
};

#define VCMD_net_migrate	VC_CMD(NETMIG, 1, 0)

#define VCMD_net_add		VC_CMD(NETALT, 1, 0)
#define VCMD_net_remove		VC_CMD(NETALT, 2, 0)

struct	vcmd_net_addr_v0 {
	uint16_t type;
	uint16_t count;
	uint32_t ip[4];
	uint32_t mask[4];
	/* more to come */
};




/* flag commands */

#define VCMD_get_nflags		VC_CMD(FLAGS, 5, 0)
#define VCMD_set_nflags		VC_CMD(FLAGS, 6, 0)

struct	vcmd_net_flags_v0 {
	uint64_t flagword;
	uint64_t mask;
};



/* network caps commands */

#define VCMD_get_ncaps		VC_CMD(FLAGS, 7, 0)
#define VCMD_set_ncaps		VC_CMD(FLAGS, 8, 0)

struct	vcmd_net_caps_v0 {
	uint64_t ncaps;
	uint64_t cmask;
};

#endif	/* _VX_CONTEXT_CMD_H */
