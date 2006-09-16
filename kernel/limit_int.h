#ifndef _VX_LIMIT_INT_H
#define _VX_LIMIT_INT_H


#ifdef	__KERNEL__

#define VXD_RCRES(r)	VXD_CBIT(cres, (r))
#define VXD_RLIMIT(r)	VXD_CBIT(limit, (r))

extern const char *vlimit_name[NUM_LIMITS];

static inline void __vx_acc_cres(struct vx_info *vxi,
	int res, int dir, void *_data, char *_file, int _line)
{
	if (VXD_RCRES(res))
		vxlprintk(1, "vx_acc_cres[%5d,%s,%2d]: %5d%s (%p)",
			(vxi ? vxi->vx_id : -1), vlimit_name[res], res,
			(vxi ? atomic_read(&vxi->limit.rcur[res]) : 0),
			(dir > 0) ? "++" : "--", _data, _file, _line);
	if (!vxi)
		return;

	if (dir > 0)
		atomic_inc(&vxi->limit.rcur[res]);
	else
		atomic_dec(&vxi->limit.rcur[res]);
}

static inline void __vx_add_cres(struct vx_info *vxi,
	int res, int amount, void *_data, char *_file, int _line)
{
	if (VXD_RCRES(res))
		vxlprintk(1, "vx_add_cres[%5d,%s,%2d]: %5d += %5d (%p)",
			(vxi ? vxi->vx_id : -1), vlimit_name[res], res,
			(vxi ? atomic_read(&vxi->limit.rcur[res]) : 0),
			amount, _data, _file, _line);
	if (amount == 0)
		return;
	if (!vxi)
		return;
	atomic_add(amount, &vxi->limit.rcur[res]);
}

static inline int __vx_cres_avail(struct vx_info *vxi,
		int res, int num, char *_file, int _line)
{
	unsigned long value;

	if (VXD_RLIMIT(res))
		vxlprintk(1, "vx_cres_avail[%5d,%s,%2d]: %5ld > %5d + %5d",
			(vxi ? vxi->vx_id : -1), vlimit_name[res], res,
			(vxi ? vxi->limit.rlim[res] : 1),
			(vxi ? atomic_read(&vxi->limit.rcur[res]) : 0),
			num, _file, _line);
	if (num == 0)
		return 1;
	if (!vxi)
		return 1;

	value = atomic_read(&vxi->limit.rcur[res]);

	if (value > vxi->limit.rmax[res])
		vxi->limit.rmax[res] = value;

	if (vxi->limit.rlim[res] == RLIM_INFINITY)
		return 1;

	if (value + num <= vxi->limit.rlim[res])
		return 1;

	atomic_inc(&vxi->limit.lhit[res]);
	return 0;
}

#endif	/* __KERNEL__ */
#endif	/* _VX_LIMIT_H */
