#ifndef _VX_DEBUG_CMD_H
#define _VX_DEBUG_CMD_H


/* debug commands */

#define VCMD_dump_history	VC_CMD(DEBUG, 1, 0)

#ifdef	__KERNEL__

extern int vc_dump_history(uint32_t);

#endif	/* __KERNEL__ */
#endif	/* _VX_DEBUG_CMD_H */
