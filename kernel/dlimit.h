#ifndef _VX_DLIMIT_H
#define _VX_DLIMIT_H

#include "switch.h"

#define CDLIM_UNSET		(0ULL)
#define CDLIM_INFINITY		(~0ULL)
#define CDLIM_KEEP		(~1ULL)


#ifdef	__KERNEL__

#include <linux/spinlock.h>

struct super_block;

struct dl_info {
	struct hlist_node dl_hlist;		/* linked list of contexts */
	struct rcu_head dl_rcu;			/* the rcu head */
	xid_t dl_xid;				/* context id */
	atomic_t dl_usecnt;			/* usage count */
	atomic_t dl_refcnt;			/* reference count */

	struct super_block *dl_sb;		/* associated superblock */

	spinlock_t dl_lock;			/* protect the values */

	uint64_t dl_space_used;			/* used space in bytes */
	uint64_t dl_space_total;		/* maximum space in bytes */
	uint32_t dl_inodes_used;		/* used inodes */
	uint32_t dl_inodes_total;		/* maximum inodes */

	unsigned int dl_nrlmult;		/* non root limit mult */
};

struct rcu_head;

extern void rcu_free_dl_info(struct rcu_head *);
extern void unhash_dl_info(struct dl_info *);

extern struct dl_info *locate_dl_info(struct super_block *, xid_t);


struct kstatfs;

extern void vx_vsi_statfs(struct super_block *, struct kstatfs *);

typedef uint64_t dlsize_t;

#endif	/* __KERNEL__ */
#else	/* _VX_DLIMIT_H */
#warning duplicate inclusion
#endif	/* _VX_DLIMIT_H */
