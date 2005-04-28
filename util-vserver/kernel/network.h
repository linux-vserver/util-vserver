#ifndef _VX_NETWORK_H
#define _VX_NETWORK_H

#include <linux/types.h>


#define MAX_N_CONTEXT	65535	/* Arbitrary limit */

#define NX_DYNAMIC_ID	((uint32_t)-1)		/* id for dynamic context */

#define NB_IPV4ROOT	16


#ifdef	__KERNEL__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <asm/atomic.h>


struct nx_info {
	struct hlist_node nx_hlist;	/* linked list of nxinfos */
	struct rcu_head nx_rcu;		/* the rcu head */
	nid_t nx_id;			/* vnet id */
	atomic_t nx_usecnt;		/* usage count */
	atomic_t nx_refcnt;		/* reference count */

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


struct rcu_head;

extern void unhash_nx_info(struct nx_info *);

extern struct nx_info *locate_nx_info(int);
extern struct nx_info *locate_or_create_nx_info(int);

extern int get_nid_list(int, unsigned int *, int);
extern int nid_is_hashed(nid_t);

extern int nx_migrate_task(struct task_struct *, struct nx_info *);

struct in_ifaddr;
struct net_device;

int ifa_in_nx_info(struct in_ifaddr *, struct nx_info *);
int dev_in_nx_info(struct net_device *, struct nx_info *);

struct sock;

int nx_addr_conflict(struct nx_info *, uint32_t, struct sock *);

#endif	/* __KERNEL__ */
#else	/* _VX_NETWORK_H */
#warning duplicate inclusion
#endif	/* _VX_NETWORK_H */
