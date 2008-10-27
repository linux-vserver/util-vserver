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

uint_least64_t
vc_get_insecurebcaps()
{
  return ~((1<<VC_CAP_CHOWN) | (1<<VC_CAP_DAC_OVERRIDE) |
	   (1<<VC_CAP_DAC_READ_SEARCH) | (1<<VC_CAP_FOWNER) |
	   (1<<VC_CAP_FSETID) | (1<<VC_CAP_KILL) |
	   (1<<VC_CAP_SETGID) | (1<<VC_CAP_SETUID) |
	   (1<<VC_CAP_NET_BIND_SERVICE)

#if defined(VC_ENABLE_API_COMPAT)
	   | (vc_isSupported(vcFEATURE_VSHELPER) ? (1<<VC_CAP_SYS_BOOT) : 0)
	   | (vc_isSupported(vcFEATURE_MIGRATE)  ? (1<<VC_CAP_AUDIT_WRITE) : 0) // formerly QUOTACTL
#endif
    );
}
