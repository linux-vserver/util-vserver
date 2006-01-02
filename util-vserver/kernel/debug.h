#ifndef _VX_DEBUG_H
#define _VX_DEBUG_H

#ifndef CONFIG_VSERVER
#warning config options missing
#endif

#define VXD_CBIT(n,m)	(vx_debug_ ## n & (1 << (m)))
#define VXD_CMIN(n,m)	(vx_debug_ ## n > (m))
#define VXD_MASK(n,m)	(vx_debug_ ## n & (m))

#define VXD_QPOS(v,p)	(((uint32_t)(v) >> ((p)*8)) & 0xFF)
#define VXD_QUAD(v)	VXD_QPOS(v,0), VXD_QPOS(v,1),		\
			VXD_QPOS(v,2), VXD_QPOS(v,3)
#define VXF_QUAD	"%u.%u.%u.%u"

#define VXD_DEV(d)	(d), (d)->bd_inode->i_ino,		\
			imajor((d)->bd_inode), iminor((d)->bd_inode)
#define VXF_DEV		"%p[%lu,%d:%d]"


#define __FUNC__	__func__


#ifdef	CONFIG_VSERVER_DEBUG

extern unsigned int vx_debug_switch;
extern unsigned int vx_debug_xid;
extern unsigned int vx_debug_nid;
extern unsigned int vx_debug_net;
extern unsigned int vx_debug_limit;
extern unsigned int vx_debug_dlim;
extern unsigned int vx_debug_cvirt;
extern unsigned int vx_debug_misc;


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

#define VXH_HERE(__type)			\
	({ __label__ __vxh_##__type;		\
		__vxh_##__type:;		\
		&&__vxh_##__type; })



static inline void __vxh_copy_vxi(struct _vx_hist_entry *entry, struct vx_info *vxi)
{
	entry->vxi.ptr = vxi;
	if (vxi) {
		entry->vxi.usecnt = atomic_read(&vxi->vx_usecnt);
		entry->vxi.tasks = atomic_read(&vxi->vx_tasks);
		entry->vxi.xid = vxi->vx_id;
	}
}


#define __VXH_BODY(__type, __data)		\
	struct _vx_hist_entry *entry;		\
						\
	preempt_disable();			\
	entry = vxh_advance(VXH_HERE(__type));	\
	__data;					\
	entry->type = __type;			\
	preempt_enable();


	/* pass vxi only */
#define __VXH_SIMPLE				\
	__vxh_copy_vxi(entry, vxi)

#define VXH_SIMPLE(__name, __type)		\
static inline void __name(struct vx_info *vxi)	\
{						\
	__VXH_BODY(__type, __VXH_SIMPLE)	\
}

	/* pass vxi and data (void *) */
#define __VXH_DATA				\
	__vxh_copy_vxi(entry, vxi);		\
	entry->sc.data = data

#define VXH_DATA(__name, __type)		\
static inline					\
void __name(struct vx_info *vxi, void *data)	\
{						\
	__VXH_BODY(__type, __VXH_DATA)		\
}

	/* pass vxi and arg (long) */
#define __VXH_LARG				\
	__vxh_copy_vxi(entry, vxi);		\
	entry->ll.arg = arg

#define VXH_LARG(__name, __type)		\
static inline					\
void __name(struct vx_info *vxi, long arg)	\
{						\
	__VXH_BODY(__type, __VXH_LARG)		\
}


static inline void vxh_throw_oops(void)
{
	__VXH_BODY(VXH_THROW_OOPS, {});
	/* prevent further acquisition */
	vxh_active = 0;
}

VXH_SIMPLE(vxh_get_vx_info,	VXH_GET_VX_INFO);
VXH_SIMPLE(vxh_put_vx_info,	VXH_PUT_VX_INFO);

VXH_DATA(vxh_init_vx_info,	VXH_INIT_VX_INFO);
VXH_DATA(vxh_set_vx_info,	VXH_SET_VX_INFO);
VXH_DATA(vxh_clr_vx_info,	VXH_CLR_VX_INFO);

VXH_DATA(vxh_claim_vx_info,	VXH_CLAIM_VX_INFO);
VXH_DATA(vxh_release_vx_info,	VXH_RELEASE_VX_INFO);

VXH_SIMPLE(vxh_alloc_vx_info,	VXH_ALLOC_VX_INFO);
VXH_SIMPLE(vxh_dealloc_vx_info, VXH_DEALLOC_VX_INFO);

VXH_SIMPLE(vxh_hash_vx_info,	VXH_HASH_VX_INFO);
VXH_SIMPLE(vxh_unhash_vx_info,	VXH_UNHASH_VX_INFO);

VXH_LARG(vxh_loc_vx_info,	VXH_LOC_VX_INFO);
VXH_LARG(vxh_lookup_vx_info,	VXH_LOOKUP_VX_INFO);
VXH_LARG(vxh_create_vx_info,	VXH_CREATE_VX_INFO);

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
#define vxd_assert(c,f,x...)	vxlprintk(!(c), \
	"assertion [" f "] failed.", ##x, __FILE__, __LINE__)
#else
#define vxd_assert_lock(l)	do { } while (0)
#define vxd_assert(c,f,x...)	do { } while (0)
#endif


#endif /* _VX_DEBUG_H */
