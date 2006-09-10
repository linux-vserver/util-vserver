#ifndef _VX_LEGACY_H
#define _VX_LEGACY_H

#include "switch.h"

/*  compatibiliy vserver commands */

#define VCMD_new_s_context	VC_CMD(COMPAT, 1, 1)
#define VCMD_set_ipv4root	VC_CMD(COMPAT, 2, 3)

#define VCMD_create_context	VC_CMD(VSETUP, 1, 0)

/*  compatibiliy vserver arguments */

struct	vcmd_new_s_context_v1 {
	uint32_t remove_cap;
	uint32_t flags;
};

struct	vcmd_set_ipv4root_v3 {
	/* number of pairs in id */
	uint32_t broadcast;
	struct {
		uint32_t ip;
		uint32_t mask;
	} nx_mask_pair[NB_IPV4ROOT];
};


#define VX_INFO_LOCK		1	/* Can't request a new vx_id */
#define VX_INFO_NPROC		4	/* Limit number of processes in a context */
#define VX_INFO_PRIVATE		8	/* Noone can join this security context */
#define VX_INFO_INIT		16	/* This process wants to become the */
					/* logical process 1 of the security */
					/* context */
#define VX_INFO_HIDEINFO	32	/* Hide some information in /proc */
#define VX_INFO_ULIMIT		64	/* Use ulimit of the current process */
					/* to become the global limits */
					/* of the context */
#define VX_INFO_NAMESPACE	128	/* save private namespace */


#ifdef	__KERNEL__
extern int vc_new_s_context(uint32_t, void __user *);
extern int vc_set_ipv4root(uint32_t, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_LEGACY_H */
