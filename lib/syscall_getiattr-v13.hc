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

static inline ALWAYSINLINE int
vc_get_iattr_v13(char const *filename,
		 xid_t *xid, uint32_t *flags, uint32_t *mask)
{
  struct vcmd_ctx_iattr_v1	attr;
  int				rc;

  attr.name = filename;
  
  rc = vserver(VCMD_get_iattr, 0, &attr);
  if (xid)   *xid   = CTX_KERNEL2USER(attr.xid);
  if (flags) *flags = attr.flags;
  if (mask)  *mask  = attr.mask;

  return rc;
}
