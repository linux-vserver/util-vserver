#ifndef _VX_NETWORK_H
#define _VX_NETWORK_H


#define NB_IPV4ROOT	16

#ifdef	__KERNEL__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/utsname.h>
#include <asm/resource.h>
#include <asm/atomic.h>


struct ip_info {
	struct list_head ip_list;		/* linked list of ipinfos */
	atomic_t ip_refcount;
	int nbipv4;
	__u32 ipv4[NB_IPV4ROOT];/* Process can only bind to these IPs */
				/* The first one is used to connect */
				/* and for bind any service */
				/* The other must be used explicity when */
				/* binding */
	__u32 mask[NB_IPV4ROOT];/* Netmask for each ipv4 */
				/* Used to select the proper source address */
				/* for sockets */
	__u32 v4_bcast;	/* Broadcast address used to receive UDP packets */
};


extern spinlock_t iplist_lock;
extern struct list_head ip_infos;


void free_ip_info(struct ip_info *);
struct ip_info *create_ip_info(void);

#endif	/* __KERNEL__ */
#endif	/* _VX_NETWORK_H */
