#ifndef _VX_NETWORK_H
#define _VX_NETWORK_H

#include <linux/types.h>


#define MAX_N_CONTEXT	65535	/* Arbitrary limit */

#define NB_IPV4ROOT	16


/* network flags */

#define NXF_INFO_PRIVATE	0x00000008

#define NXF_SINGLE_IP		0x00000100

#define NXF_HIDE_NETIF		0x02000000

#define NXF_STATE_SETUP		(1ULL << 32)
#define NXF_STATE_ADMIN		(1ULL << 34)

#define NXF_SC_HELPER		(1ULL << 36)
#define NXF_PERSISTENT		(1ULL << 38)

#define NXF_ONE_TIME		(0x0005ULL << 32)

#define NXF_INIT_SET		(NXF_STATE_ADMIN)


/* address types */

#define NXA_TYPE_IPV4		0x0001
#define NXA_TYPE_IPV6		0x0002

#define NXA_TYPE_NONE		0x0000
#define NXA_TYPE_ANY		0x00FF

#define NXA_TYPE_ADDR		0x0003
#define NXA_TYPE_MASK		0x0013
#define NXA_TYPE_RANGE		0x0023

#define NXA_MOD_BCAST		0x0100
#define NXA_MOD_LBACK		0x0200


#else	/* _VX_NETWORK_H */
#warning duplicate inclusion
#endif	/* _VX_NETWORK_H */
