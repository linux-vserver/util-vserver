// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

static inline ALWAYSINLINE int
vc_set_dlimit_v13(char const *filename, xid_t xid,
		  uint_least32_t flags, struct vc_ctx_dlimit const *limits)
{
  struct vcmd_ctx_dlimit_v0	attr = {
    .name		= filename,
    .flags		= flags,
    .space_used		= CDLIM_USER2KERNEL(limits->space_used),
    .space_total	= CDLIM_USER2KERNEL(limits->space_total),
    .inodes_used	= CDLIM_USER2KERNEL(limits->inodes_used),
    .inodes_total	= CDLIM_USER2KERNEL(limits->inodes_total),
    .reserved		= CDLIM_USER2KERNEL(limits->reserved),
  };
  
  return vserver(VCMD_set_dlimit, CTX_USER2KERNEL(xid), &attr);
}
