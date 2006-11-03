#ifndef _VX_CVIRT_CMD_H
#define _VX_CVIRT_CMD_H


/* virtual host info name commands */

#define VCMD_set_vhi_name	VC_CMD(VHOST, 1, 0)
#define VCMD_get_vhi_name	VC_CMD(VHOST, 2, 0)

struct	vcmd_vhi_name_v0 {
	uint32_t field;
	char name[65];
};


enum vhi_name_field {
	VHIN_CONTEXT=0,
	VHIN_SYSNAME,
	VHIN_NODENAME,
	VHIN_RELEASE,
	VHIN_VERSION,
	VHIN_MACHINE,
	VHIN_DOMAINNAME,
};


#ifdef	__KERNEL__

#include <linux/compiler.h>

extern int vc_set_vhi_name(struct vx_info *, void __user *);
extern int vc_get_vhi_name(struct vx_info *, void __user *);

#endif	/* __KERNEL__ */

#define VCMD_virt_stat		VC_CMD(VSTAT, 3, 0)

struct	vcmd_virt_stat_v0 {
	uint64_t offset;
	uint64_t uptime;
	uint32_t nr_threads;
	uint32_t nr_running;
	uint32_t nr_uninterruptible;
	uint32_t nr_onhold;
	uint32_t nr_forks;
	uint32_t load[3];
};

#ifdef	__KERNEL__
extern int vc_virt_stat(struct vx_info *, void __user *);

#endif	/* __KERNEL__ */
#endif	/* _VX_CVIRT_CMD_H */
