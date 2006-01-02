#ifndef _VX_XID_H
#define _VX_XID_H

#ifndef CONFIG_VSERVER
#warning config options missing
#endif

#define XID_TAG(in)	(IS_TAGXID(in))


#ifdef CONFIG_XID_TAG_NFSD
#define XID_TAG_NFSD	1
#else
#define XID_TAG_NFSD	0
#endif


#ifdef CONFIG_INOXID_NONE

#define MAX_UID		0xFFFFFFFF
#define MAX_GID		0xFFFFFFFF

#define INOXID_XID(tag, uid, gid, xid)	(0)

#define XIDINO_UID(tag, uid, xid)	(uid)
#define XIDINO_GID(tag, gid, xid)	(gid)

#endif


#ifdef CONFIG_INOXID_GID16

#define MAX_UID		0xFFFFFFFF
#define MAX_GID		0x0000FFFF

#define INOXID_XID(tag, uid, gid, xid)	\
	((tag) ? (((gid) >> 16) & 0xFFFF) : 0)

#define XIDINO_UID(tag, uid, xid)	(uid)
#define XIDINO_GID(tag, gid, xid)	\
	((tag) ? (((gid) & 0xFFFF) | ((xid) << 16)) : (gid))

#endif


#ifdef CONFIG_INOXID_UGID24

#define MAX_UID		0x00FFFFFF
#define MAX_GID		0x00FFFFFF

#define INOXID_XID(tag, uid, gid, xid)	\
	((tag) ? ((((uid) >> 16) & 0xFF00) | (((gid) >> 24) & 0xFF)) : 0)

#define XIDINO_UID(tag, uid, xid)	\
	((tag) ? (((uid) & 0xFFFFFF) | (((xid) & 0xFF00) << 16)) : (uid))
#define XIDINO_GID(tag, gid, xid)	\
	((tag) ? (((gid) & 0xFFFFFF) | (((xid) & 0x00FF) << 24)) : (gid))

#endif


#ifdef CONFIG_INOXID_UID16

#define MAX_UID		0x0000FFFF
#define MAX_GID		0xFFFFFFFF

#define INOXID_XID(tag, uid, gid, xid)	\
	((tag) ? (((uid) >> 16) & 0xFFFF) : 0)

#define XIDINO_UID(tag, uid, xid)	\
	((tag) ? (((uid) & 0xFFFF) | ((xid) << 16)) : (uid))
#define XIDINO_GID(tag, gid, xid)	(gid)

#endif


#ifdef CONFIG_INOXID_INTERN

#define MAX_UID		0xFFFFFFFF
#define MAX_GID		0xFFFFFFFF

#define INOXID_XID(tag, uid, gid, xid)	\
	((tag) ? (xid) : 0)

#define XIDINO_UID(tag, uid, xid)	(uid)
#define XIDINO_GID(tag, gid, xid)	(gid)

#endif


#ifdef CONFIG_INOXID_RUNTIME

#define MAX_UID		0xFFFFFFFF
#define MAX_GID		0xFFFFFFFF

#define INOXID_XID(tag, uid, gid, xid)	(0)

#define XIDINO_UID(tag, uid, xid)	(uid)
#define XIDINO_GID(tag, gid, xid)	(gid)

#endif


#ifndef CONFIG_INOXID_NONE
#define vx_current_fsxid(sb)	\
	((sb)->s_flags & MS_TAGXID ? current->xid : 0)
#else
#define vx_current_fsxid(sb)	(0)
#endif

#ifndef CONFIG_INOXID_INTERN
#define XIDINO_XID(tag, xid)	(0)
#else
#define XIDINO_XID(tag, xid)	((tag) ? (xid) : 0)
#endif

#define INOXID_UID(tag, uid, gid)	\
	((tag) ? ((uid) & MAX_UID) : (uid))
#define INOXID_GID(tag, uid, gid)	\
	((tag) ? ((gid) & MAX_GID) : (gid))


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

int vx_parse_xid(char *string, xid_t *xid, int remove);
void vx_propagate_xid(struct nameidata *nd, struct inode *inode);

#endif /* _VX_XID_H */
