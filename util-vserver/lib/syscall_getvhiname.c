// $Id$    --*- c -*--

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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.h"
#include "linuxvirtual.h"

#if defined(VC_ENABLE_API_OLDUTS) && defined(VC_ENABLE_API_V13)
#  define VC_MULTIVERSION_SYSCALL	1
#endif
#include "vserver-internal.h"

#ifdef VC_ENABLE_API_V13
#  include "syscall_getvhiname-v13.hc"
#endif

#ifdef VC_ENABLE_API_OLDUTS
#  include "syscall_getvhiname-olduts.hc"
#endif

int
vc_get_vhi_name(xid_t xid, vc_uts_type type, char *val, size_t len)
{
  CALL_VC(CALL_VC_V13   (vc_get_vhi_name, xid, type, val, len),
	  CALL_VC_OLDUTS(vc_get_vhi_name, xid, type, val, len));
}
