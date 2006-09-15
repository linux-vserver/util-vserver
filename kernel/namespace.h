#ifndef _VX_NAMESPACE_H
#define _VX_NAMESPACE_H


#include <linux/types.h>

struct vx_info;
struct namespace;
struct fs_struct;

extern int vx_set_namespace(struct vx_info *, struct namespace *, struct fs_struct *);

#else	/* _VX_NAMESPACE_H */
#warning duplicate inclusion
#endif	/* _VX_NAMESPACE_H */
