// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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


#ifndef H_UTIL_VSERVER_SRC_WRAPPERS_VSERVER_H
#define H_UTIL_VSERVER_SRC_WRAPPERS_VSERVER_H

#include "wrappers.h"
#include <vserver.h>

inline static UNUSED void
Evc_new_s_context(ctx_t ctx, unsigned int remove_cap, unsigned int flags)
{
  FatalErrnoError(vc_new_s_context(ctx,remove_cap,flags)==-1,
		  "vc_new_s_context()");
}

inline static UNUSED ctx_t
Evc_X_getctx(pid_t pid)
{
  register ctx_t	res = vc_X_getctx(pid);
  FatalErrnoError(res==VC_NOCTX, "vc_X_getctx()");
  return res;
}

#endif	//  H_UTIL_VSERVER_SRC_WRAPPERS_VSERVER_H
