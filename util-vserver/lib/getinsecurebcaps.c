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
  return ( (1<<VC_CAP_LINUX_IMMUTABLE) | (1<<VC_CAP_NET_BROADCAST) |
	   (1<<VC_CAP_NET_ADMIN) | (1<<VC_CAP_NET_RAW) |
	   (1<<VC_CAP_IPC_LOCK) | (1<<VC_CAP_IPC_OWNER) |
	   (1<<VC_CAP_SYS_MODULE) | (1<<VC_CAP_SYS_RAWIO) |
	   (1<<VC_CAP_SYS_PACCT) | (1<<VC_CAP_SYS_ADMIN) |
	   (1<<VC_CAP_SYS_NICE) |
	   (1<<VC_CAP_SYS_RESOURCE) | (1<<VC_CAP_SYS_TIME) |
	   (1<<VC_CAP_MKNOD) | (1<<VC_CAP_QUOTACTL)

#if defined(VC_ENABLE_API_COMPAT)
	   | (vc_isSupported(vcFEATURE_VSHELPER) ? 0 : (1<<VC_CAP_SYS_BOOT))
#endif
    );
}
