#ifndef _VX_NETWORK_H
#define _VX_NETWORK_H

#define MAX_N_CONTEXT	65535	/* Arbitrary limit */

#define IP_DYNAMIC_ID	((uint32_t)-1)		/* id for dynamic context */

#define NB_IPV4ROOT	16

#ifdef	__KERNEL__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/utsname.h>
#include <asm/resource.h>
#include <asm/atomic.h>


struct nx_info {
	struct list_head nx_list;	/* linked list of nxinfos */
	nid_t nx_id;			/* vnet id */
	atomic_t nx_refcount;

	uint64_t nx_flags;		/* network flag word */
	uint64_t nx_ncaps;		/* network capabilities */

	int nbipv4;
	__u32 ipv4[NB_IPV4ROOT];	/* Process can only bind to these IPs */
					/* The first one is used to connect */
					/* and for bind any service */
					/* The other must be used explicity */
	__u32 mask[NB_IPV4ROOT];	/* Netmask for each ipv4 */
					/* Used to select the proper source */
					/* address for sockets */
	__u32 v4_bcast;			/* Broadcast address to receive UDP  */

	char nx_name[65];		/* network context name */
};


extern spinlock_t nxlist_lock;
extern struct list_head nx_infos;


void free_nx_info(struct nx_info *);
struct nx_info *create_nx_info(void);

extern struct nx_info *find_nx_info(int);
extern int nx_info_id_valid(int);

struct in_ifaddr;
struct net_device;

int ifa_in_nx_info(struct in_ifaddr *, struct nx_info *);
int dev_in_nx_info(struct net_device *, struct nx_info *);


#endif	/* __KERNEL__ */

#include "switch.h"

/* vinfo commands */

#define VCMD_task_nid		VC_CMD(VINFO, 2, 0)

#ifdef	__KERNEL__
extern int vc_task_nid(uint32_t, void __user *);

#endif	/* __KERNEL__ */

#define VCMD_nx_info		VC_CMD(VINFO, 6, 0)

struct  vcmd_nx_info_v0 {
	uint32_t nid;
	/* more to come */	
};

#ifdef	__KERNEL__
extern int vc_nx_info(uint32_t, void __user *);

#endif	/* __KERNEL__ */

#define VCMD_net_create		VC_CMD(VNET, 1, 0)
#define VCMD_net_migrate	VC_CMD(NETMIG, 1, 0)

#define VCMD_net_add		VC_CMD(NETALT, 1, 0)
#define VCMD_net_remove		VC_CMD(NETALT, 2, 0)

struct  vcmd_net_nx_v0 {
	uint16_t type;
	uint16_t count;
	uint32_t ip[4];
	uint32_t mask[4];
	/* more to come */	
};

//	IPN_TYPE_IPV4	


#ifdef	__KERNEL__
extern int vc_net_create(uint32_t, void __user *);
extern int vc_net_migrate(uint32_t, void __user *);

#endif	/* __KERNEL__ */

#define VCMD_get_nflags		VC_CMD(FLAGS, 5, 0)
#define VCMD_set_nflags		VC_CMD(FLAGS, 6, 0)

struct  vcmd_net_flags_v0 {
	uint64_t flagword;
	uint64_t mask;
};

#ifdef	__KERNEL__
extern int vc_get_nflags(uint32_t, void __user *);
extern int vc_set_nflags(uint32_t, void __user *);

#endif	/* __KERNEL__ */

#define IPF_STATE_SETUP		(1ULL<<32)


#define IPF_ONE_TIME		(0x0001ULL<<32)

#define VCMD_get_ncaps		VC_CMD(FLAGS, 7, 0)
#define VCMD_set_ncaps		VC_CMD(FLAGS, 8, 0)

struct  vcmd_net_caps_v0 {
	uint64_t ncaps;
	uint64_t cmask;
};

#ifdef	__KERNEL__
extern int vc_get_ncaps(uint32_t, void __user *);
extern int vc_set_ncaps(uint32_t, void __user *);

#endif	/* __KERNEL__ */

#define IPC_WOSSNAME		0x00000001


#endif	/* _VX_NETWORK_H */
