#ifndef _VX_SCHED_CMD_H
#define _VX_SCHED_CMD_H


/*  sched vserver commands */

#define VCMD_set_sched_v2	VC_CMD(SCHED, 1, 2)
#define VCMD_set_sched_v3	VC_CMD(SCHED, 1, 3)
#define VCMD_set_sched		VC_CMD(SCHED, 1, 4)

struct	vcmd_set_sched_v2 {
	int32_t fill_rate;
	int32_t interval;
	int32_t tokens;
	int32_t tokens_min;
	int32_t tokens_max;
	uint64_t cpu_mask;
};

struct	vcmd_set_sched_v3 {
	uint32_t set_mask;
	int32_t fill_rate;
	int32_t interval;
	int32_t tokens;
	int32_t tokens_min;
	int32_t tokens_max;
	int32_t priority_bias;
};

struct	vcmd_set_sched_v4 {
	uint32_t set_mask;
	int32_t fill_rate;
	int32_t interval;
	int32_t tokens;
	int32_t tokens_min;
	int32_t tokens_max;
	int32_t prio_bias;
	int32_t cpu_id;
	int32_t bucket_id;
};


#define VXSM_FILL_RATE		0x0001
#define VXSM_INTERVAL		0x0002
#define VXSM_FILL_RATE2		0x0004
#define VXSM_INTERVAL2		0x0008
#define VXSM_TOKENS		0x0010
#define VXSM_TOKENS_MIN		0x0020
#define VXSM_TOKENS_MAX		0x0040
#define VXSM_PRIO_BIAS		0x0100

#define VXSM_IDLE_TIME		0x0200
#define VXSM_FORCE		0x0400

#define	VXSM_V3_MASK		0x0173
#define	VXSM_SET_MASK		0x01FF

#define VXSM_CPU_ID		0x1000
#define VXSM_BUCKET_ID		0x2000

#define SCHED_KEEP		(-2)	/* only for v2 */

#ifdef	__KERNEL__

#include <linux/compiler.h>

extern int vc_set_sched_v2(struct vx_info *, void __user *);
extern int vc_set_sched_v3(struct vx_info *, void __user *);
extern int vc_set_sched(struct vx_info *, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_SCHED_CMD_H */
