#ifndef _LINUX_VIRTUAL_H
#define _LINUX_VIRTUAL_H

#define VC_CATEGORY(c)		(((c) >> 24) & 0x3F)
#define VC_COMMAND(c)     	(((c) >> 16) & 0xFF)
#define VC_VERSION(c)		((c) & 0xFFF)

#define VC_CMD(c,i,v)    	((((VC_CAT_ ## c) & 0x3F) << 24) \
    	    	    	    	| (((i) & 0xFF) << 16) | ((v) & 0xFFF))

#define	VC_CAT_VERSION		0
#define	VC_CAT_PROCESS		1
#define	VC_CAT_MEMORY		2
#define	VC_CAT_NETWORK		3

#define	VC_CAT_LIMITS		8
#define	VC_CAT_QUOTA		9

#define	VC_CAT_OTHER		62
#define	VC_CAT_COMPAT		63

/*  interface version */

  //#define VC_VERSION  	    	0x00010000



/*  query version */

#define VCMD_get_version  	VC_CMD(VERSION, 0, 0)


/*  compatibiliy vserver commands */

#define VCMD_new_s_context  	VC_CMD(COMPAT, 1, 1)
#define VCMD_set_ipv4root	VC_CMD(COMPAT, 2, 3)

/*  compatibiliy vserver arguments */

struct  vcmd_new_s_context_v1 {
    	uint32_t remove_cap;
	uint32_t flags;
};

#define	NB_IPV4ROOT 16

struct  vcmd_set_ipv4root_v3 {
    	/* number of pairs in id */
    	uint32_t broadcast;
	struct {
		uint32_t ip;
		uint32_t mask;
	} ip_mask_pair[NB_IPV4ROOT];
};


#endif /* _LINUX_VIRTUAL_H */
