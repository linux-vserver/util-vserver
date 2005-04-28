#ifndef _VX_DEBUG_H
#define _VX_DEBUG_H

#ifndef	CONFIG_VSERVER
#warning config options missing
#endif

#define VXD_CBIT(n,m)	(vx_debug_ ## n & (1 << (m)))
#define VXD_CMIN(n,m)	(vx_debug_ ## n > (m))
#define VXD_MASK(n,m)	(vx_debug_ ## n & (m))

#define VXD_QPOS(v,p)	(((uint32_t)(v) >> ((p)*8)) & 0xFF)
#define VXD_QUAD(v)	VXD_QPOS(v,0), VXD_QPOS(v,1),		\
			VXD_QPOS(v,2), VXD_QPOS(v,3)

#define __FUNC__	__func__


#ifdef	CONFIG_VSERVER_DEBUG

extern unsigned int vx_debug_switch;
extern unsigned int vx_debug_xid;
extern unsigned int vx_debug_nid;
extern unsigned int vx_debug_net;
extern unsigned int vx_debug_limit;
extern unsigned int vx_debug_dlim;
extern unsigned int vx_debug_cvirt;


#define VX_LOGLEVEL	"vxD: "
#define VX_WARNLEVEL	KERN_WARNING "vxW: "

#define vxdprintk(c,f,x...)					\
	do {							\
		if (c)						\
			printk(VX_LOGLEVEL f "\n" , ##x);	\
	} while (0)

#define vxlprintk(c,f,x...)					\
	do {							\
		if (c)						\
			printk(VX_LOGLEVEL f " @%s:%d\n", x);	\
	} while (0)

#define vxfprintk(c,f,x...)					\
	do {							\
		if (c)						\
			printk(VX_LOGLEVEL f " %s@%s:%d\n", x); \
	} while (0)


#define vxwprintk(c,f,x...)					\
	do {							\
		if (c)						\
			printk(VX_WARNLEVEL f "\n" , ##x);	\
	} while (0)


#define vxd_path(d,m)						\
	({ static char _buffer[PATH_MAX];			\
	   d_path((d), (m), _buffer, sizeof(_buffer)); })

#else	/* CONFIG_VSERVER_DEBUG */

#define vx_debug_switch 0
#define vx_debug_xid	0
#define vx_debug_nid	0
#define vx_debug_net	0
#define vx_debug_limit	0
#define vx_debug_dlim	0
#define vx_debug_cvirt	0

#define vxdprintk(x...) do { } while (0)
#define vxlprintk(x...) do { } while (0)
#define vxfprintk(x...) do { } while (0)
#define vxwprintk(x...) do { } while (0)

#define vxd_path	"<none>"

#endif	/* CONFIG_VSERVER_DEBUG */


/* history stuff */

#ifdef	CONFIG_VSERVER_HISTORY


extern unsigned volatile int vxh_active;

struct _vxhe_vxi {
	struct vx_info *ptr;
	unsigned xid;
	unsigned usecnt;
	unsigned tasks;
};

struct _vxhe_set_clr {
	void *data;
};

struct _vxhe_loc_lookup {
	unsigned arg;
};

enum {
	VXH_UNUSED=0,
	VXH_THROW_OOPS=1,

	VXH_GET_VX_INFO,
	VXH_PUT_VX_INFO,
	VXH_INIT_VX_INFO,
	VXH_SET_VX_INFO,
	VXH_CLR_VX_INFO,
	VXH_CLAIM_VX_INFO,
	VXH_RELEASE_VX_INFO,
	VXH_ALLOC_VX_INFO,
	VXH_DEALLOC_VX_INFO,
	VXH_HASH_VX_INFO,
	VXH_UNHASH_VX_INFO,
	VXH_LOC_VX_INFO,
	VXH_LOOKUP_VX_INFO,
	VXH_CREATE_VX_INFO,
};

struct _vx_hist_entry {
	void *loc;
	unsigned short seq;
	unsigned short type;
	struct _vxhe_vxi vxi;
	union {
		struct _vxhe_set_clr sc;
		struct _vxhe_loc_lookup ll;
	};
};

struct _vx_hist_entry *vxh_advance(void *loc);

#define	VXH_HERE()		\
	({ __label__ here;	\
		here:;		\
		&&here; })



static inline void __vxh_copy_vxi(struct _vx_hist_entry *entry, struct vx_info *vxi)
{
	entry->vxi.ptr = vxi;
	if (vxi) {
		entry->vxi.usecnt = atomic_read(&vxi->vx_usecnt);
		entry->vxi.tasks = atomic_read(&vxi->vx_tasks);
		entry->vxi.xid = vxi->vx_id;
	}
}

static inline void vxh_throw_oops(void)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	entry->type = VXH_THROW_OOPS;
	preempt_enable();

	/* prevent further acquisition */
	vxh_active = 0;
}

static inline void vxh_get_vx_info(struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->type = VXH_GET_VX_INFO;
	preempt_enable();
}

static inline void vxh_put_vx_info(struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->type = VXH_PUT_VX_INFO;
	preempt_enable();
}

static inline void vxh_init_vx_info(struct vx_info *vxi, void *data)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->sc.data = data;
	entry->type = VXH_INIT_VX_INFO;
	preempt_enable();
}

static inline void vxh_set_vx_info(struct vx_info *vxi, void *data)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->sc.data = data;
	entry->type = VXH_SET_VX_INFO;
	preempt_enable();
}

static inline void vxh_clr_vx_info(struct vx_info *vxi, void *data)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->sc.data = data;
	entry->type = VXH_CLR_VX_INFO;
	preempt_enable();
}

static inline void vxh_claim_vx_info(struct vx_info *vxi, void *data)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->sc.data = data;
	entry->type = VXH_CLAIM_VX_INFO;
	preempt_enable();
}

static inline void vxh_release_vx_info(struct vx_info *vxi, void *data)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->sc.data = data;
	entry->type = VXH_RELEASE_VX_INFO;
	preempt_enable();
}

static inline void vxh_alloc_vx_info(struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->type = VXH_ALLOC_VX_INFO;
	preempt_enable();
}

static inline void vxh_dealloc_vx_info(struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->type = VXH_DEALLOC_VX_INFO;
	preempt_enable();
}

static inline void vxh_hash_vx_info(struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->type = VXH_HASH_VX_INFO;
	preempt_enable();
}

static inline void vxh_unhash_vx_info(struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->type = VXH_UNHASH_VX_INFO;
	preempt_enable();
}

static inline void vxh_loc_vx_info(unsigned arg, struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->ll.arg = arg;
	entry->type = VXH_LOC_VX_INFO;
	preempt_enable();
}

static inline void vxh_lookup_vx_info(unsigned arg, struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->ll.arg = arg;
	entry->type = VXH_LOOKUP_VX_INFO;
	preempt_enable();
}

static inline void vxh_create_vx_info(unsigned arg, struct vx_info *vxi)
{
	struct _vx_hist_entry *entry;

	preempt_disable();
	entry = vxh_advance(VXH_HERE());
	__vxh_copy_vxi(entry, vxi);
	entry->ll.arg = arg;
	entry->type = VXH_CREATE_VX_INFO;
	preempt_enable();
}


extern void vxh_dump_history(void);

#else  /* CONFIG_VSERVER_HISTORY */

#define vxh_throw_oops()		do { } while (0)

#define vxh_get_vx_info(v)		do { } while (0)
#define vxh_put_vx_info(v)		do { } while (0)

#define vxh_init_vx_info(v,d)		do { } while (0)
#define vxh_set_vx_info(v,d)		do { } while (0)
#define vxh_clr_vx_info(v,d)		do { } while (0)

#define vxh_claim_vx_info(v,d)		do { } while (0)
#define vxh_release_vx_info(v,d)	do { } while (0)

#define vxh_alloc_vx_info(v)		do { } while (0)
#define vxh_dealloc_vx_info(v)		do { } while (0)

#define vxh_hash_vx_info(v)		do { } while (0)
#define vxh_unhash_vx_info(v)		do { } while (0)

#define vxh_loc_vx_info(a,v)		do { } while (0)
#define vxh_lookup_vx_info(a,v)		do { } while (0)
#define vxh_create_vx_info(a,v)		do { } while (0)

#define vxh_dump_history()		do { } while (0)


#endif /* CONFIG_VSERVER_HISTORY */


#ifdef	CONFIG_VSERVER_DEBUG
#define vxd_assert_lock(l)	assert_spin_locked(l)
#else
#define	vxd_assert_lock(l)	do { } while (0)
#endif


#endif /* _VX_DEBUG_H */
