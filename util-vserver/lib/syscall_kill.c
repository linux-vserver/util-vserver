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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.h"
#include "vserver-internal.h"
#include "linuxvirtual.h"

#if defined(VC_ENABLE_API_V11) || defined(VC_ENABLE_API_V13)
#  include "syscall_kill-v11.hc"
#endif

#ifdef VC_ENABLE_API_V13
#  define vc_ctx_kill_v13	vc_ctx_kill_v11
#endif


#if defined(VC_ENABLE_API_V11) || defined(VC_ENABLE_API_V13)

  // NOTICE: the reverse order of V11 -> V13 is correct here since these are
  // the same syscalls

int
vc_ctx_kill(xid_t ctx, pid_t pid, int sig)
{
  CALL_VC(CALL_VC_V11(vc_ctx_kill, ctx, pid, sig),
	  CALL_VC_V13(vc_ctx_kill, ctx, pid, sig));
}

#endif
