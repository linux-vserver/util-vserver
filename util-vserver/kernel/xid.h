#ifndef _LINUX_XID_H_
#define _LINUX_XID_H_

#ifdef CONFIG_INOXID_NONE

#define MAX_UID		0xFFFFFFFF
#define MAX_GID		0xFFFFFFFF

#define INOXID_XID(uid, gid, xid)	(0)

#define XIDINO_UID(uid, xid)		(uid)
#define XIDINO_GID(gid, xid)		(gid)

#endif


#ifdef CONFIG_INOXID_GID16

#define MAX_UID		0xFFFFFFFF
#define MAX_GID		0x0000FFFF

#define INOXID_XID(uid, gid, xid)	(((gid) >> 16) & 0xFFFF)

#define XIDINO_UID(uid, xid)		(uid)
#define XIDINO_GID(gid, xid)		(((gid) & 0xFFFF) | ((xid) << 16))


#endif


#ifdef CONFIG_INOXID_GID24

#define MAX_UID		0x00FFFFFF
#define MAX_GID		0x00FFFFFF

#define INOXID_XID(uid, gid, xid)	((((uid) >> 16) & 0xFF00) | (((gid) >> 24) & 0xFF))

#define XIDINO_UID(uid, xid)		(((uid) & 0xFFFFFF) | (((xid) & 0xFF00) << 16))
#define XIDINO_GID(gid, xid)		(((gid) & 0xFFFFFF) | (((xid) & 0x00FF) << 24))

#endif


#ifdef CONFIG_INOXID_GID32

#define MAX_UID		0xFFFFFFFF
#define MAX_GID		0xFFFFFFFF

#define INOXID_XID(uid, gid, xid)	(xid)

#define XIDINO_UID(uid, xid)		(uid)
#define XIDINO_GID(gid, xid)		(gid)

#endif


#ifdef CONFIG_INOXID_RUNTIME

#define MAX_UID		0xFFFFFFFF
#define MAX_GID		0xFFFFFFFF

#define INOXID_XID(uid, gid, xid)	(0)

#define XIDINO_UID(uid, xid)		(uid)
#define XIDINO_GID(gid, xid)		(gid)

#endif


#define INOXID_UID(uid, gid)		((uid) & MAX_UID)
#define INOXID_GID(uid, gid)		((gid) & MAX_GID)

static inline uid_t vx_map_uid(uid_t uid)
{
	if ((uid > MAX_UID) && (uid != -1))
		uid = -2;
	return (uid & MAX_UID);
}

static inline gid_t vx_map_gid(gid_t gid)
{
	if ((gid > MAX_GID) && (gid != -1))
		gid = -2;
	return (gid & MAX_GID);
}


#ifdef	CONFIG_VSERVER_LEGACY		
#define FIOC_GETXID	_IOR('x', 1, long)
#define FIOC_SETXID	_IOW('x', 2, long)
#define FIOC_SETXIDJ	_IOW('x', 3, long)
#endif

#endif /* _LINUX_XID_H_ */
