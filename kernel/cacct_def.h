#ifndef _VX_CACCT_DEF_H
#define _VX_CACCT_DEF_H

#include <asm/atomic.h>
#include <linux/vserver/cacct.h>


struct _vx_sock_acc {
	atomic_t count;
	atomic_t total;
};

/* context sub struct */

struct _vx_cacct {
	struct _vx_sock_acc sock[VXA_SOCK_SIZE][3];
	atomic_t slab[8];
	atomic_t page[6][8];
};

#ifdef CONFIG_VSERVER_DEBUG

static inline void __dump_vx_cacct(struct _vx_cacct *cacct)
{
	int i,j;

	printk("\t_vx_cacct:");
	for (i=0; i<6; i++) {
		struct _vx_sock_acc *ptr = cacct->sock[i];

		printk("\t [%d] =", i);
		for (j=0; j<3; j++) {
			printk(" [%d] = %8d, %8d", j,
				atomic_read(&ptr[j].count),
				atomic_read(&ptr[j].total));
		}
		printk("\n");
	}
}

#endif

#endif	/* _VX_CACCT_DEF_H */
