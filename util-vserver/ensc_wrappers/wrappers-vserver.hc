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

#include <vserver.h>

inline static WRAPPER_DECL xid_t
Evc_new_s_context(xid_t ctx, unsigned int remove_cap, unsigned int flags)
{
  register xid_t	res = vc_new_s_context(ctx,remove_cap,flags);
  FatalErrnoError(res==VC_NOCTX, "vc_new_s_context()");
  return res;
}

inline static WRAPPER_DECL xid_t
Evc_get_task_xid(pid_t pid)
{
  register xid_t	res = vc_get_task_xid(pid);
  FatalErrnoError(res==VC_NOCTX, "vc_get_task_xid()");
  return res;
}

inline static WRAPPER_DECL xid_t
Evc_create_context(xid_t xid)
{
  register xid_t	res = vc_create_context(xid);
  FatalErrnoError(res==VC_NOCTX, "vc_create_context()");
  return res;
}

inline static WRAPPER_DECL void
Evc_migrate_context(xid_t xid)
{
  FatalErrnoError(vc_migrate_context(xid)==-1, "vc_migrate_context()");
}

inline static WRAPPER_DECL void
Evc_get_flags(xid_t xid, struct vc_ctx_flags *flags)
{
  FatalErrnoError(vc_get_flags(xid, flags)!=-1, "vc_get_flags()");
}

inline static WRAPPER_DECL void
Evc_set_flags(xid_t xid, struct vc_ctx_flags const *flags)
{
  FatalErrnoError(vc_set_flags(xid, flags)!=-1, "vc_get_flags()");
}
