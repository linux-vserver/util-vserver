#ifndef _LINUX_VIRTUAL_H
#define _LINUX_VIRTUAL_H

#define VC_CATEGORY(c)		(((c) >> 24) & 0x3F)
#define VC_COMMAND(c)		(((c) >> 16) & 0xFF)
#define VC_VERSION(c)		((c) & 0xFFF)

#define VC_CMD(c,i,v)		((((VC_CAT_ ## c) & 0x3F) << 24) \
				| (((i) & 0xFF) << 16) | ((v) & 0xFFF))

/*

  Syscall Matrix V2.2

         |VERSION|CREATE |MODIFY |MIGRATE|CONTROL|EXPERIM| |SPECIAL|SPECIAL|
         |STATS  |DESTROY|ALTER  |CHANGE |LIMIT  |TEST   | |       |       |
         |INFO   |SETUP  |       |MOVE   |       |       | |       |       |
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  SYSTEM |VERSION|       |       |       |       |       | |DEVICES|       |
  HOST   |     00|     01|     02|     03|     04|     05| |     06|     07|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  CPU    |       |       |       |       |       |       | |SCHED. |       |
  PROCESS|     08|     09|     10|     11|     12|     13| |     14|     15|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  MEMORY |       |       |       |       |       |       | |SWAP   |       |
         |     16|     17|     18|     19|     20|     21| |     22|     23|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  NETWORK|       |       |       |       |       |       | |SERIAL |       |
         |     24|     25|     26|     27|     28|     29| |     30|     31|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  DISK   |       |       |       |       |       |       | |       |       |
  VFS    |     32|     33|     34|     35|     36|     37| |     38|     39|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  OTHER  |       |       |       |       |       |       | |       |       |
         |     40|     41|     42|     43|     44|     45| |     46|     47|
  =======+=======+=======+=======+=======+=======+=======+ +=======+=======+
  SPECIAL|       |       |       |       |       |       | |       |       |
         |     48|     49|     50|     51|     52|     53| |     54|     55|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  SPECIAL|       |       |       |       |RLIMIT |SYSCALL| |       |COMPAT |
         |     56|     57|     58|     59|     60|TEST 61| |     62|     63|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+

*/

#define	VC_CAT_VERSION		0

#define VC_CAT_PROCTRL		12

#define VC_CAT_RLIMIT		60

#define VC_CAT_SYSTEST		61
#define	VC_CAT_COMPAT		63
	
/*  interface version */

#define VCI_VERSION		0x00010001



/*  query version */

#define VCMD_get_version	VC_CMD(VERSION, 0, 0)


/*  compatibiliy vserver commands */

#define VCMD_new_s_context	VC_CMD(COMPAT, 1, 1)
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

/*  context signalling */

#define VCMD_ctx_kill		VC_CMD(PROCTRL, 1, 0)

struct  vcmd_ctx_kill_v0 {
	int32_t pid;
	int32_t sig;
};

/*  rlimit vserver commands */

#define VCMD_get_rlimit	    	VC_CMD(RLIMIT, 1, 0)
#define VCMD_set_rlimit	    	VC_CMD(RLIMIT, 2, 0)
#define VCMD_get_rlimit_mask  	VC_CMD(RLIMIT, 3, 0)

struct  vcmd_ctx_rlimit_v0 {
    	uint32_t id;
	uint64_t minimum;
	uint64_t softlimit;
	uint64_t maximum;
};

struct  vcmd_ctx_rlimit_mask_v0 {
	uint32_t minimum;
	uint32_t softlimit;
	uint32_t maximum;
};

#define CRLIM_INFINITY	    	(~0ULL)
#define CRLIM_KEEP	    	(~1ULL)


#endif /* _LINUX_VIRTUAL_H */
