#ifndef _VX_MONITOR_H
#define _VX_MONITOR_H


enum {
	VXM_UNUSED = 0,

	VXM_SYNC = 0x10,

	VXM_UPDATE = 0x20,
	VXM_UPDATE_1,
	VXM_UPDATE_2,

	VXM_RQINFO_1 = 0x24,
	VXM_RQINFO_2,

	VXM_ACTIVATE = 0x40,
	VXM_DEACTIVATE,
	VXM_IDLE,

	VXM_HOLD = 0x44,
	VXM_UNHOLD,

	VXM_MIGRATE = 0x48,
	VXM_RESCHED,

	/* all other bits are flags */
	VXM_SCHED = 0x80,
};

struct _vxm_update_1 {
	uint32_t tokens_max;
	uint32_t fill_rate;
	uint32_t interval;
};

struct _vxm_update_2 {
	uint32_t tokens_min;
	uint32_t fill_rate;
	uint32_t interval;
};

struct _vxm_rqinfo_1 {
	uint16_t running;
	uint16_t onhold;
	uint16_t iowait;
	uint16_t uintr;
	uint32_t idle_tokens;
};

struct _vxm_rqinfo_2 {
	uint32_t norm_time;
	uint32_t idle_time;
	uint32_t idle_skip;
};

struct _vxm_sched {
	uint32_t tokens;
	uint32_t norm_time;
	uint32_t idle_time;
};

struct _vxm_task {
	uint16_t pid;
	uint16_t state;
};

struct _vxm_event {
	uint32_t jif;
	union {
		uint32_t seq;
		uint32_t sec;
	};
	union {
		uint32_t tokens;
		uint32_t nsec;
		struct _vxm_task tsk;
	};
};

struct _vx_mon_entry {
	uint16_t type;
	uint16_t xid;
	union {
		struct _vxm_event ev;
		struct _vxm_sched sd;
		struct _vxm_update_1 u1;
		struct _vxm_update_2 u2;
		struct _vxm_rqinfo_1 q1;
		struct _vxm_rqinfo_2 q2;
	};
};


#endif /* _VX_MONITOR_H */
