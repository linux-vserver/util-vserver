#ifndef _VX_GLOBAL_H
#define _VX_GLOBAL_H

#ifndef	CONFIG_VSERVER
#warning config options missing
#endif


extern atomic_t vx_global_ctotal;
extern atomic_t vx_global_cactive;

#endif /* _VX_GLOBAL_H */
