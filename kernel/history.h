#ifndef _VX_HISTORY_H
#define _VX_HISTORY_H


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

#ifdef	CONFIG_VSERVER_HISTORY

extern unsigned volatile int vxh_active;

struct _vx_hist_entry *vxh_advance(void *loc);


static inline
void	__vxh_copy_vxi(struct _vx_hist_entry *entry, struct vx_info *vxi)
{
	entry->vxi.ptr = vxi;
	if (vxi) {
		entry->vxi.usecnt = atomic_read(&vxi->vx_usecnt);
		entry->vxi.tasks = atomic_read(&vxi->vx_tasks);
		entry->vxi.xid = vxi->vx_id;
	}
}


#define	__HERE__ current_text_addr()

#define __VXH_BODY(__type, __data, __here)	\
	struct _vx_hist_entry *entry;		\
						\
	preempt_disable();			\
	entry = vxh_advance(__here);		\
	__data;					\
	entry->type = __type;			\
	preempt_enable();


	/* pass vxi only */

#define __VXH_SMPL				\
	__vxh_copy_vxi(entry, vxi)

static inline
void	__vxh_smpl(struct vx_info *vxi, int __type, void *__here)
{
	__VXH_BODY(__type, __VXH_SMPL, __here)
}

	/* pass vxi and data (void *) */

#define __VXH_DATA				\
	__vxh_copy_vxi(entry, vxi);		\
	entry->sc.data = data

static inline
void	__vxh_data(struct vx_info *vxi, void *data,
			int __type, void *__here)
{
	__VXH_BODY(__type, __VXH_DATA, __here)
}

	/* pass vxi and arg (long) */

#define __VXH_LONG				\
	__vxh_copy_vxi(entry, vxi);		\
	entry->ll.arg = arg

static inline
void	__vxh_long(struct vx_info *vxi, long arg,
			int __type, void *__here)
{
	__VXH_BODY(__type, __VXH_LONG, __here)
}


static inline
void	__vxh_throw_oops(void *__here)
{
	__VXH_BODY(VXH_THROW_OOPS, {}, __here);
	/* prevent further acquisition */
	vxh_active = 0;
}


#define vxh_throw_oops()	__vxh_throw_oops(__HERE__);

#define __vxh_get_vx_info(v,h)	__vxh_smpl(v, VXH_GET_VX_INFO, h);
#define __vxh_put_vx_info(v,h)	__vxh_smpl(v, VXH_PUT_VX_INFO, h);

#define __vxh_init_vx_info(v,d,h) \
	__vxh_data(v,d, VXH_INIT_VX_INFO, h);
#define __vxh_set_vx_info(v,d,h) \
	__vxh_data(v,d, VXH_SET_VX_INFO, h);
#define __vxh_clr_vx_info(v,d,h) \
	__vxh_data(v,d, VXH_CLR_VX_INFO, h);

#define __vxh_claim_vx_info(v,d,h) \
	__vxh_data(v,d, VXH_CLAIM_VX_INFO, h);
#define __vxh_release_vx_info(v,d,h) \
	__vxh_data(v,d, VXH_RELEASE_VX_INFO, h);

#define vxh_alloc_vx_info(v) \
	__vxh_smpl(v, VXH_ALLOC_VX_INFO, __HERE__);
#define vxh_dealloc_vx_info(v) \
	__vxh_smpl(v, VXH_DEALLOC_VX_INFO, __HERE__);

#define vxh_hash_vx_info(v) \
	__vxh_smpl(v, VXH_HASH_VX_INFO, __HERE__);
#define vxh_unhash_vx_info(v) \
	__vxh_smpl(v, VXH_UNHASH_VX_INFO, __HERE__);

#define vxh_loc_vx_info(v,l) \
	__vxh_long(v,l, VXH_LOC_VX_INFO, __HERE__);
#define vxh_lookup_vx_info(v,l) \
	__vxh_long(v,l, VXH_LOOKUP_VX_INFO, __HERE__);
#define vxh_create_vx_info(v,l) \
	__vxh_long(v,l, VXH_CREATE_VX_INFO, __HERE__);

extern void vxh_dump_history(void);


#else  /* CONFIG_VSERVER_HISTORY */

#define	__HERE__	0

#define vxh_throw_oops()		do { } while (0)

#define __vxh_get_vx_info(v,h)		do { } while (0)
#define __vxh_put_vx_info(v,h)		do { } while (0)

#define __vxh_init_vx_info(v,d,h)	do { } while (0)
#define __vxh_set_vx_info(v,d,h)	do { } while (0)
#define __vxh_clr_vx_info(v,d,h)	do { } while (0)

#define __vxh_claim_vx_info(v,d,h)	do { } while (0)
#define __vxh_release_vx_info(v,d,h)	do { } while (0)

#define vxh_alloc_vx_info(v)		do { } while (0)
#define vxh_dealloc_vx_info(v)		do { } while (0)

#define vxh_hash_vx_info(v)		do { } while (0)
#define vxh_unhash_vx_info(v)		do { } while (0)

#define vxh_loc_vx_info(a,v)		do { } while (0)
#define vxh_lookup_vx_info(a,v)		do { } while (0)
#define vxh_create_vx_info(a,v)		do { } while (0)

#define vxh_dump_history()		do { } while (0)


#endif /* CONFIG_VSERVER_HISTORY */

#endif /* _VX_HISTORY_H */
