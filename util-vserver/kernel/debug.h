#ifndef _VX_DEBUG_H
#define _VX_DEBUG_H


#define VXD_CBIT(n,m)	(vx_debug_ ## n & (1 << (m)))
#define VXD_CMIN(n,m)	(vx_debug_ ## n > (m))
#define VXD_MASK(n,m)	(vx_debug_ ## n & (m))

#define VXD_QPOS(v,p)	(((uint32_t)(v) >> ((p)*8)) & 0xFF)
#define VXD_QUAD(v)	VXD_QPOS(v,0), VXD_QPOS(v,1),		\
			VXD_QPOS(v,2), VXD_QPOS(v,3)

// #define	VXD_HERE	__FILE__, __LINE__

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

#define vxdprintk(c,f,x...)					\
	do {							\
		if (c)						\
			printk(VX_LOGLEVEL f "\n", x);		\
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

#else

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

#endif



#endif /* _VX_DEBUG_H */
