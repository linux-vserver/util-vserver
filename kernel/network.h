#ifndef _VX_NETWORK_H
#define _VX_NETWORK_H

#include <linux/types.h>


#define MAX_N_CONTEXT	65535	/* Arbitrary limit */

#define NX_DYNAMIC_ID	((uint32_t)-1)		/* id for dynamic context */

#define NB_IPV4ROOT	16


/* network flags */

#define NXF_INFO_LOCK		0x00000001

#define NXF_STATE_SETUP		(1ULL<<32)
#define NXF_STATE_ADMIN		(1ULL<<34)

#define NXF_SC_HELPER		(1ULL<<36)
#define NXF_PERSISTENT		(1ULL<<38)

#define NXF_ONE_TIME		(0x0005ULL<<32)

#define NXF_INIT_SET		(NXF_STATE_ADMIN)


/* address types */

#define NXA_TYPE_IPV4		1
#define NXA_TYPE_IPV6		2

#define NXA_MOD_BCAST		(1<<8)

#define NXA_TYPE_ANY		((uint16_t)-1)


#else	/* _VX_NETWORK_H */
#warning duplicate inclusion
#endif	/* _VX_NETWORK_H */
