#ifndef _VX_CACCT_INT_H
#define _VX_CACCT_INT_H


#ifdef	__KERNEL__

static inline
unsigned long vx_sock_count(struct _vx_cacct *cacct, int type, int pos)
{
	return atomic_read(&cacct->sock[type][pos].count);
}


static inline
unsigned long vx_sock_total(struct _vx_cacct *cacct, int type, int pos)
{
	return atomic_read(&cacct->sock[type][pos].total);
}

#endif	/* __KERNEL__ */
#endif	/* _VX_CACCT_INT_H */
