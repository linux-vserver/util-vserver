#ifndef _LINUX_VXID_H_
#define _LINUX_VXID_H_

#ifdef CONFIG_INOXID_NONE

#define	INOXID_UID(uid, gid)		(uid)
#define	INOXID_GID(uid, gid)		(gid)
#define	INOXID_XID(uid, gid, xid)	(0)

#define	XIDINO_UID(uid, xid)		(uid)
#define	XIDINO_GID(gid, xid)		(gid)

#define	MAX_UID		0xFFFFFFFF
#define	MAX_GID		0xFFFFFFFF

#endif

#ifdef CONFIG_INOXID_GID16

#define	INOXID_UID(uid, gid)		(uid)
#define	INOXID_GID(uid, gid)		((gid) & 0xFFFF)
#define	INOXID_XID(uid, gid, xid)	(((gid) >> 16) & 0xFFFF)

#define	XIDINO_UID(uid, xid)		(uid)
#define	XIDINO_GID(gid, xid)		(((gid) & 0xFFFF) | ((xid) << 16))

#define	MAX_UID		0xFFFFFFFF
#define	MAX_GID		0x0000FFFF

#endif

#ifdef CONFIG_INOXID_GID24

#define	INOXID_UID(uid, gid)		((uid) & 0xFFFFFF)
#define	INOXID_GID(uid, gid)		((gid) & 0xFFFFFF)
#define	INOXID_XID(uid, gid, xid)	((((uid) >> 16) & 0xFF00) | (((gid) >> 24) & 0xFF))

#define	XIDINO_UID(uid, xid)		(((uid) & 0xFFFFFF) | (((xid) & 0xFF00) << 16))
#define	XIDINO_GID(gid, xid)		(((gid) & 0xFFFFFF) | (((xid) & 0x00FF) << 24))

#define	MAX_UID		0x00FFFFFF
#define	MAX_GID		0x00FFFFFF

#endif

#ifdef CONFIG_INOXID_GID32

#define	INOXID_UID(uid, gid)		(uid)
#define	INOXID_GID(uid, gid)		(gid)
#define	INOXID_XID(uid, gid, xid)	(xid)

#define	XIDINO_UID(uid, xid)		(uid)
#define	XIDINO_GID(gid, xid)		(gid)

#define	MAX_UID		0xFFFFFFFF
#define	MAX_GID		0xFFFFFFFF

#endif

#define	FIOC_GETXID	_IOR('x', 1, long)
#define	FIOC_SETXID	_IOW('x', 2, long)
#define	FIOC_SETXIDJ	_IOW('x', 3, long)

#endif
