#ifndef _VX_CONTEXT_H
#define _VX_CONTEXT_H

#include <linux/types.h>

#define MAX_S_CONTEXT	65535	/* Arbitrary limit */
#define MIN_D_CONTEXT	49152	/* dynamic contexts start here */

#define VX_DYNAMIC_ID	((uint32_t)-1)		/* id for dynamic context */

#ifdef	__KERNEL__

#include <linux/list.h>
#include <linux/spinlock.h>

#define	_VX_INFO_DEF_
#include "cvirt.h"
#include "limit.h"
#include "sched.h"
#undef	_VX_INFO_DEF_

struct vx_info {
	struct list_head vx_list;		/* linked list of contexts */
	xid_t vx_id;				/* context id */
	atomic_t vx_refcount;			/* refcount */
	struct vx_info *vx_parent;		/* parent context */

	struct namespace *vx_namespace;		/* private namespace */
	struct fs_struct *vx_fs;		/* private namespace fs */
	uint64_t vx_flags;			/* VX_INFO_xxx */
	uint64_t vx_bcaps;			/* bounding caps (system) */
	uint64_t vx_ccaps;			/* context caps (vserver) */

	pid_t vx_initpid;			/* PID of fake init process */

	struct _vx_cvirt cvirt;			/* virtual/bias stuff */
	struct _vx_limit limit;			/* vserver limits */
	struct _vx_sched sched;			/* vserver scheduler */

	char vx_name[65];			/* vserver name */
};


extern spinlock_t vxlist_lock;
extern struct list_head vx_infos;


#define	VX_ADMIN	0x0001
#define	VX_WATCH	0x0002
#define VX_DUMMY	0x0008

#define	VX_IDENT	0x0010
#define	VX_EQUIV	0x0020
#define	VX_PARENT	0x0040
#define	VX_CHILD	0x0080

#define	VX_ARG_MASK	0x00F0

#define	VX_DYNAMIC	0x0100
#define	VX_STATIC	0x0200

#define	VX_ATR_MASK	0x0F00


void free_vx_info(struct vx_info *);

extern struct vx_info *find_vx_info(int);
extern struct vx_info *find_or_create_vx_info(int);
extern int vx_info_id_valid(int);

extern int vx_migrate_task(struct task_struct *, struct vx_info *);

#endif	/* __KERNEL__ */

#include "switch.h"

/* vinfo commands */

#define	VCMD_task_xid		VC_CMD(VINFO, 1, 0)
#define	VCMD_task_nid		VC_CMD(VINFO, 2, 0)

#ifdef	__KERNEL__
extern int vc_task_xid(uint32_t, void *);

#endif	/* __KERNEL__ */

#define	VCMD_vx_info		VC_CMD(VINFO, 5, 0)
#define	VCMD_nx_info		VC_CMD(VINFO, 6, 0)

struct  vcmd_vx_info_v0 {
	uint32_t xid;
	uint32_t initpid;
	/* more to come */	
};

#ifdef	__KERNEL__
extern int vc_vx_info(uint32_t, void *);

#endif	/* __KERNEL__ */

#define VCMD_create_context	VC_CMD(VSETUP, 1, 0)
#define VCMD_migrate_context	VC_CMD(PROCMIG, 1, 0)

#ifdef	__KERNEL__
extern int vc_create_context(uint32_t, void *);
extern int vc_migrate_context(uint32_t, void *);

#endif	/* __KERNEL__ */

#define VCMD_get_flags		VC_CMD(FLAGS, 1, 0)
#define VCMD_set_flags		VC_CMD(FLAGS, 2, 0)

struct  vcmd_ctx_flags_v0 {
	uint64_t flagword;
	uint64_t mask;
};

#ifdef	__KERNEL__
extern int vc_get_flags(uint32_t, void *);
extern int vc_set_flags(uint32_t, void *);

#endif	/* __KERNEL__ */

#define VXF_INFO_LOCK		0x00000001
#define VXF_INFO_SCHED		0x00000002
#define VXF_INFO_NPROC		0x00000004
#define VXF_INFO_PRIVATE	0x00000008

#define VXF_INFO_INIT		0x00000010
#define VXF_INFO_HIDE		0x00000020
#define VXF_INFO_ULIMIT		0x00000040
#define VXF_INFO_NSPACE		0x00000080

#define	VXF_SCHED_HARD		0x00000100
#define	VXF_SCHED_PRIO		0x00000200
#define	VXF_SCHED_PAUSE		0x00000400

#define VXF_VIRT_MEM		0x00010000
#define VXF_VIRT_UPTIME		0x00020000
#define VXF_VIRT_CPU		0x00040000

#define VXF_HIDE_MOUNT		0x01000000
#define VXF_HIDE_NETIF		0x02000000

#define	VXF_STATE_SETUP		(1ULL<<32)
#define	VXF_STATE_INIT		(1ULL<<33)


#define	VXF_ONE_TIME		(0x0003ULL<<32)

#define VCMD_get_ccaps		VC_CMD(FLAGS, 3, 0)
#define VCMD_set_ccaps		VC_CMD(FLAGS, 4, 0)

struct  vcmd_ctx_caps_v0 {
	uint64_t bcaps;
	uint64_t ccaps;
	uint64_t cmask;
};

#ifdef	__KERNEL__
extern int vc_get_ccaps(uint32_t, void *);
extern int vc_set_ccaps(uint32_t, void *);

#endif	/* __KERNEL__ */

#define VXC_SET_UTSNAME		0x00000001
#define VXC_SET_RLIMIT		0x00000002

#define VXC_ICMP_PING		0x00000100

#define VXC_SECURE_MOUNT	0x00010000


#endif	/* _VX_CONTEXT_H */
