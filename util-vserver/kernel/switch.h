#ifndef _VX_SWITCH_H
#define _VX_SWITCH_H

#include <linux/types.h>

#define VC_CATEGORY(c)		(((c) >> 24) & 0x3F)
#define VC_COMMAND(c)		(((c) >> 16) & 0xFF)
#define VC_VERSION(c)		((c) & 0xFFF)

#define VC_CMD(c,i,v)		((((VC_CAT_ ## c) & 0x3F) << 24) \
				| (((i) & 0xFF) << 16) | ((v) & 0xFFF))

/*

  Syscall Matrix V2.5

         |VERSION|CREATE |MODIFY |MIGRATE|CONTROL|EXPERIM| |SPECIAL|SPECIAL|
         |STATS  |DESTROY|ALTER  |CHANGE |LIMIT  |TEST   | |       |       |
         |INFO   |SETUP  |       |MOVE   |       |       | |       |       |
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  SYSTEM |VERSION|       |       |       |       |       | |DEVICES|       |
  HOST   |     00|     01|     02|     03|     04|     05| |     06|     07|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  CPU    |       |       |PROCALT|PROCMIG|PROCTRL|       | |SCHED. |       |
  PROCESS|     08|     09|     10|     11|     12|     13| |     14|     15|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  MEMORY |       |       |       |       |       |       | |SWAP   |       |
         |     16|     17|     18|     19|     20|     21| |     22|     23|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  NETWORK|       |       |       |       |       |       | |SERIAL |       |
         |     24|     25|     26|     27|     28|     29| |     30|     31|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  DISK   |       |       |       |       |       |       | |INODE  |       |
  VFS    |     32|     33|     34|     35|     36|     37| |     38|     39|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  OTHER  |       |       |       |       |       |       | |VINFO  |       |
         |     40|     41|     42|     43|     44|     45| |     46|     47|
  =======+=======+=======+=======+=======+=======+=======+ +=======+=======+
  SPECIAL|       |       |       |       |FLAGS  |       | |       |       |
         |     48|     49|     50|     51|     52|     53| |     54|     55|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+
  SPECIAL|       |       |       |       |RLIMIT |SYSCALL| |       |COMPAT |
         |     56|     57|     58|     59|     60|TEST 61| |     62|     63|
  -------+-------+-------+-------+-------+-------+-------+ +-------+-------+

*/

#define VC_CAT_VERSION		0

#define VC_CAT_VSETUP		1
#define	VC_CAT_VHOST		2
	
#define	VC_CAT_PROCALT		10
#define	VC_CAT_PROCMIG		11
#define VC_CAT_PROCTRL		12

#define VC_CAT_SCHED		14
#define VC_CAT_INODE		38

#define	VC_CAT_VINFO		46

#define VC_CAT_FLAGS		52
#define VC_CAT_RLIMIT		60

#define VC_CAT_SYSTEST		61
#define VC_CAT_COMPAT		63
	
/*  interface version */

#define VCI_VERSION		0x00010014


/*  query version */

#define VCMD_get_version	VC_CMD(VERSION, 0, 0)


#ifdef	__KERNEL__

#include <linux/errno.h>

#define	ENOTSUP		-EOPNOTSUPP

#else	/* __KERNEL__ */
#define	__user
#endif	/* __KERNEL__ */

#endif	/* _VX_SWITCH_H */
