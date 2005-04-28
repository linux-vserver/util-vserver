#ifndef _VX_LIMIT_CMD_H
#define _VX_LIMIT_CMD_H

/*  rlimit vserver commands */

#define VCMD_get_rlimit		VC_CMD(RLIMIT, 1, 0)
#define VCMD_set_rlimit		VC_CMD(RLIMIT, 2, 0)
#define VCMD_get_rlimit_mask	VC_CMD(RLIMIT, 3, 0)

struct	vcmd_ctx_rlimit_v0 {
	uint32_t id;
	uint64_t minimum;
	uint64_t softlimit;
	uint64_t maximum;
};

struct	vcmd_ctx_rlimit_mask_v0 {
	uint32_t minimum;
	uint32_t softlimit;
	uint32_t maximum;
};

#define CRLIM_UNSET		(0ULL)
#define CRLIM_INFINITY		(~0ULL)
#define CRLIM_KEEP		(~1ULL)

#ifdef	__KERNEL__

#include <linux/compiler.h>

extern int vc_get_rlimit(uint32_t, void __user *);
extern int vc_set_rlimit(uint32_t, void __user *);
extern int vc_get_rlimit_mask(uint32_t, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_LIMIT_CMD_H */
