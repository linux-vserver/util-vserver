#ifndef _VX_LIMIT_CMD_H
#define _VX_LIMIT_CMD_H


/*  rlimit vserver commands */

#define VCMD_get_rlimit		VC_CMD(RLIMIT, 1, 0)
#define VCMD_set_rlimit		VC_CMD(RLIMIT, 2, 0)
#define VCMD_get_rlimit_mask	VC_CMD(RLIMIT, 3, 0)
#define VCMD_reset_minmax	VC_CMD(RLIMIT, 9, 0)

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

#define VCMD_rlimit_stat	VC_CMD(VSTAT, 1, 0)

struct	vcmd_rlimit_stat_v0 {
	uint32_t id;
	uint32_t hits;
	uint64_t value;
	uint64_t minimum;
	uint64_t maximum;
};

#define CRLIM_UNSET		(0ULL)
#define CRLIM_INFINITY		(~0ULL)
#define CRLIM_KEEP		(~1ULL)

#ifdef	__KERNEL__

#ifdef	CONFIG_IA32_EMULATION

struct	vcmd_ctx_rlimit_v0_x32 {
	uint32_t id;
	uint64_t minimum;
	uint64_t softlimit;
	uint64_t maximum;
} __attribute__ ((aligned (4)));

#endif	/* CONFIG_IA32_EMULATION */

#include <linux/compiler.h>

extern int vc_get_rlimit_mask(uint32_t, void __user *);
extern int vc_get_rlimit(struct vx_info *, void __user *);
extern int vc_set_rlimit(struct vx_info *, void __user *);
extern int vc_reset_minmax(struct vx_info *, void __user *);

extern int vc_rlimit_stat(struct vx_info *, void __user *);

#ifdef	CONFIG_IA32_EMULATION

extern int vc_get_rlimit_x32(struct vx_info *, void __user *);
extern int vc_set_rlimit_x32(struct vx_info *, void __user *);

#endif	/* CONFIG_IA32_EMULATION */

#endif	/* __KERNEL__ */
#endif	/* _VX_LIMIT_CMD_H */
