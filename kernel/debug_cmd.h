#ifndef _VX_DEBUG_CMD_H
#define _VX_DEBUG_CMD_H


/* debug commands */

#define VCMD_dump_history	VC_CMD(DEBUG, 1, 0)

#define VCMD_read_history	VC_CMD(DEBUG, 5, 0)
#define VCMD_read_monitor	VC_CMD(DEBUG, 6, 0)

struct  vcmd_read_history_v0 {
	uint32_t index;
	uint32_t count;
	char __user *data;
};

struct  vcmd_read_monitor_v0 {
	uint32_t index;
	uint32_t count;
	char __user *data;
};


#ifdef	__KERNEL__

#ifdef	CONFIG_COMPAT

#include <asm/compat.h>

struct  vcmd_read_history_v0_x32 {
	uint32_t index;
	uint32_t count;
	compat_uptr_t data_ptr;
};

struct  vcmd_read_monitor_v0_x32 {
	uint32_t index;
	uint32_t count;
	compat_uptr_t data_ptr;
};

#endif  /* CONFIG_COMPAT */

extern int vc_dump_history(uint32_t);

extern int vc_read_history(uint32_t, void __user *);
extern int vc_read_monitor(uint32_t, void __user *);

#ifdef	CONFIG_COMPAT

extern int vc_read_history_x32(uint32_t, void __user *);
extern int vc_read_monitor_x32(uint32_t, void __user *);

#endif  /* CONFIG_COMPAT */

#endif	/* __KERNEL__ */
#endif	/* _VX_DEBUG_CMD_H */
