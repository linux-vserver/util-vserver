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
#include "compat.h"

#include "vserver.h"
#include "vserver-internal.h"
#include "linuxvirtual.h"

#ifdef VC_ENABLE_API_V11
#  include "syscall_rlimit-v11.hc"
#endif

#if defined (VC_ENABLE_API_V11)

int
vc_get_rlimit(xid_t ctx, int resource, struct vc_rlimit *lim)
{
  CALL_VC(CALL_VC_V11(vc_get_rlimit, ctx, resource, lim));
}

int
vc_set_rlimit(xid_t ctx, int resource, struct vc_rlimit const *lim)
{
  CALL_VC(CALL_VC_V11(vc_set_rlimit, ctx, resource, lim));
}

int
vc_get_rlimit_mask(xid_t ctx, struct vc_rlimit_mask *lim)
{
  CALL_VC(CALL_VC_V11(vc_get_rlimit_mask, ctx, 0, lim));
}
    

#endif
