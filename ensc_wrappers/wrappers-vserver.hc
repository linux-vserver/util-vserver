// $Id$    --*- c++ -*--

// Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#ifndef H_ENSC_IN_WRAPPERS_H
#  error wrappers_handler.hc can not be used in this way
#endif

#if defined(VC_ENABLE_API_COMPAT) || defined(VC_ENABLE_API_LEGACY)

inline static WRAPPER_DECL xid_t
Evc_new_s_context(xid_t ctx, unsigned int remove_cap, unsigned int flags)
{
  register xid_t	res = vc_new_s_context(ctx,remove_cap,flags);
  FatalErrnoError(res==VC_NOCTX, "vc_new_s_context()");
  return res;
}

#endif

inline static WRAPPER_DECL xid_t
Evc_get_task_xid(pid_t pid)
{
  register xid_t	res = vc_get_task_xid(pid);
  FatalErrnoError(res==VC_NOCTX, "vc_get_task_xid()");
  return res;
}

inline static WRAPPER_DECL nid_t
Evc_get_task_nid(pid_t pid)
{
  register nid_t	res = vc_get_task_nid(pid);
  FatalErrnoError(res==VC_NOCTX, "vc_get_task_nid()");
  return res;
}

inline static WRAPPER_DECL xid_t
Evc_ctx_create(xid_t xid)
{
  register xid_t	res = vc_ctx_create(xid);
  FatalErrnoError(res==VC_NOCTX, "vc_ctx_create()");
  return res;
}

inline static WRAPPER_DECL nid_t
Evc_net_create(nid_t nid)
{
  register nid_t	res = vc_net_create(nid);
  FatalErrnoError(res==VC_NOCTX, "vc_net_create()");
  return res;
}

inline static WRAPPER_DECL void
Evc_ctx_migrate(xid_t xid)
{
  FatalErrnoError(vc_ctx_migrate(xid)==-1, "vc_ctx_migrate()");
}

inline static WRAPPER_DECL void
Evc_net_migrate(nid_t nid)
{
  FatalErrnoError(vc_net_migrate(nid)==-1, "vc_net_migrate()");
}

inline static WRAPPER_DECL void
Evc_get_cflags(xid_t xid, struct vc_ctx_flags *flags)
{
  FatalErrnoError(vc_get_cflags(xid, flags)==-1, "vc_get_cflags()");
}

inline static WRAPPER_DECL void
Evc_set_cflags(xid_t xid, struct vc_ctx_flags const *flags)
{
  FatalErrnoError(vc_set_cflags(xid, flags)==-1, "vc_set_cflags()");
}

inline static WRAPPER_DECL void
Evc_get_nflags(nid_t nid, struct vc_net_flags *flags)
{
  FatalErrnoError(vc_get_nflags(nid, flags)==-1, "vc_get_nflags()");
}

inline static WRAPPER_DECL void
Evc_set_nflags(nid_t nid, struct vc_net_flags const *flags)
{
  FatalErrnoError(vc_set_nflags(nid, flags)==-1, "vc_set_nflags()");
}

inline static WRAPPER_DECL void
Evc_set_vhi_name(xid_t xid, vc_uts_type type,
		 char const *val, size_t len)
{
  FatalErrnoError(vc_set_vhi_name(xid,type,val,len)==-1, "vc_set_vhi_name()");
}

inline static WRAPPER_DECL void
Evc_get_ccaps(xid_t xid, struct vc_ctx_caps *caps)
{
  FatalErrnoError(vc_get_ccaps(xid, caps)==-1, "vc_get_ccaps()");
}

inline static WRAPPER_DECL void
Evc_set_ccaps(xid_t xid, struct vc_ctx_caps const *caps)
{
  FatalErrnoError(vc_set_ccaps(xid, caps)==-1, "vc_set_ccaps()");
}

inline static WRAPPER_DECL void
Evc_get_ncaps(nid_t nid, struct vc_net_caps *caps)
{
  FatalErrnoError(vc_get_ncaps(nid, caps)==-1, "vc_get_ncaps()");
}

inline static WRAPPER_DECL void
Evc_set_ncaps(nid_t nid, struct vc_net_caps const *caps)
{
  FatalErrnoError(vc_set_ncaps(nid, caps)==-1, "vc_set_ncaps()");
}

inline static WRAPPER_DECL void
Evc_set_namespace(xid_t xid, uint_least64_t mask)
{
  FatalErrnoError(vc_set_namespace(xid, mask)==-1, "vc_set_namespace()");
}

inline static WRAPPER_DECL void
Evc_enter_namespace(xid_t xid, uint_least64_t mask)
{
  FatalErrnoError(vc_enter_namespace(xid, mask)==-1, "vc_enter_namespace()");
}

inline static WRAPPER_DECL xid_t
Evc_xidopt2xid(char const *id, bool honor_static)
{
  char const *	err;
  xid_t		rc = vc_xidopt2xid(id, honor_static, &err);
  if (__builtin_expect(rc==VC_NOCTX,0)) {
    ENSC_DETAIL1(msg, "vc_xidopt2xid", id, 1);
#if 1
    FatalErrnoErrorFail(msg);
#else
    {
      size_t	l1 = strlen(msg);
      size_t	l2 = strlen(err);
      char	buf[l1 + l2 + sizeof(": ")];
      memcpy(buf,       msg, l1);
      memcpy(buf+l1,   ": ", 2);
      memcpy(buf+l1+2,  err, l2+1);

      FatalErrnoErrorFail(buf);
    }
#endif    
  }

  return rc;
}

inline static WRAPPER_DECL nid_t
Evc_nidopt2nid(char const *id, bool honor_static)
{
  char const *	err;
  nid_t		rc = vc_nidopt2nid(id, honor_static, &err);
  if (__builtin_expect(rc==VC_NOCTX,0)) {
    ENSC_DETAIL1(msg, "vc_nidopt2nid", id, 1);
    FatalErrnoErrorFail(msg);
  }
  return rc;
}
