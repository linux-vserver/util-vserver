#if	defined(__KERNEL__) && defined(_VX_INFO_DEF_)

#include <asm/atomic.h>
#include <asm/resource.h>

/* context sub struct */

struct _vx_limit {
	atomic_t ticks;

	unsigned long rlim[RLIM_NLIMITS];	/* Per context limit */
	atomic_t res[RLIM_NLIMITS];		/* Current value */
};

static inline void vx_info_init_limit(struct _vx_limit *limit)
{
	int lim;

	for (lim=0; lim<RLIM_NLIMITS; lim++)
		limit->rlim[lim] = RLIM_INFINITY;
}

static inline int vx_info_proc_limit(struct _vx_limit *limit, char *buffer)
{
	return sprintf(buffer,
		"PROC:\t%8d/%ld\n"
		"VM:\t%8d/%ld\n"
		"VML:\t%8d/%ld\n"		
		"RSS:\t%8d/%ld\n"
		,atomic_read(&limit->res[RLIMIT_NPROC])
		,limit->rlim[RLIMIT_NPROC]
		,atomic_read(&limit->res[RLIMIT_AS])
		,limit->rlim[RLIMIT_AS]
		,atomic_read(&limit->res[RLIMIT_MEMLOCK])
		,limit->rlim[RLIMIT_MEMLOCK]
		,atomic_read(&limit->res[RLIMIT_RSS])
		,limit->rlim[RLIMIT_RSS]
		);
}

#else	/* _VX_INFO_DEF_ */
#ifndef _VX_LIMIT_H
#define _VX_LIMIT_H

#include "switch.h"

/*  rlimit vserver commands */

#define VCMD_get_rlimit		VC_CMD(RLIMIT, 1, 0)
#define VCMD_set_rlimit		VC_CMD(RLIMIT, 2, 0)
#define VCMD_get_rlimit_mask	VC_CMD(RLIMIT, 3, 0)

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

#define CRLIM_UNSET		(0ULL)
#define CRLIM_INFINITY		(~0ULL)
#define CRLIM_KEEP		(~1ULL)

#ifdef	__KERNEL__
extern int vc_get_rlimit(uint32_t, void *);
extern int vc_set_rlimit(uint32_t, void *);
extern int vc_get_rlimit_mask(uint32_t, void *);

struct sysinfo;

void vx_vsi_meminfo(struct sysinfo *);
void vx_vsi_swapinfo(struct sysinfo *);

#endif	/* __KERNEL__ */

#endif	/* _VX_LIMIT_H */
#endif
