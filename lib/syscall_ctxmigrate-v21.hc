// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// Copyright (C) 2006 Daniel Hokka Zakrisson
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

static inline ALWAYSINLINE int
vc_ctx_migrate_spaces(xid_t xid)
{
  int ret = vc_getXIDType(xid);
  if (ret == vcTYPE_STATIC || ret == vcTYPE_DYNAMIC) {
    ret = vc_enter_namespace(xid, vc_get_space_mask() & ~(CLONE_NEWNS|CLONE_FS));
    if (ret)
      return ret;
  }

  return vserver(VCMD_ctx_migrate_v0, CTX_USER2KERNEL(xid), NULL);
}
