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

#ifdef VC_ENABLE_API_COMPAT    
#  include "syscall-compat.hc"
#endif

#ifdef VC_ENABLE_API_LEGACY
#  include "syscall-legacy.hc"
#endif

#include <stdbool.h>
#include <errno.h>

#if defined(VC_ENABLE_API_COMPAT) || defined(VC_ENABLE_API_LEGACY)

int
vc_new_s_context(ctx_t ctx, unsigned int remove_cap, unsigned int flags)
{
  CALL_VC(CALL_VC_COMPAT(vc_new_s_context, ctx, remove_cap, flags),
	  CALL_VC_LEGACY(vc_new_s_context, ctx, remove_cap, flags));
}

int
vc_set_ipv4root(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips)
{
  CALL_VC(CALL_VC_COMPAT(vc_set_ipv4root, bcast, nb, ips),
	  CALL_VC_LEGACY(vc_set_ipv4root, bcast, nb, ips));
}

#endif
