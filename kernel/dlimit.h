#ifndef _VX_DLIMIT_H
#define _VX_DLIMIT_H

#include "switch.h"


#ifdef	__KERNEL__

/*      keep in sync with CDLIM_INFINITY	*/

#define DLIM_INFINITY		(~0ULL)

#include <linux/spinlock.h>

struct super_block;

struct dl_info {
	struct hlist_node dl_hlist;		/* linked list of contexts */
	struct rcu_head dl_rcu;			/* the rcu head */
	tag_t dl_tag;				/* context tag */
	atomic_t dl_usecnt;			/* usage count */
	atomic_t dl_refcnt;			/* reference count */

	struct super_block *dl_sb;		/* associated superblock */

	spinlock_t dl_lock;			/* protect the values */

	unsigned long long dl_space_used;	/* used space in bytes */
	unsigned long long dl_space_total;	/* maximum space in bytes */
	unsigned long dl_inodes_used;		/* used inodes */
	unsigned long dl_inodes_total;		/* maximum inodes */

	unsigned int dl_nrlmult;		/* non root limit mult */
};

struct rcu_head;

extern void rcu_free_dl_info(struct rcu_head *);
extern void unhash_dl_info(struct dl_info *);

extern struct dl_info *locate_dl_info(struct super_block *, tag_t);


struct kstatfs;

extern void vx_vsi_statfs(struct super_block *, struct kstatfs *);

typedef uint64_t dlsize_t;

#endif	/* __KERNEL__ */
#else	/* _VX_DLIMIT_H */
#warning duplicate inclusion
#endif	/* _VX_DLIMIT_H */
