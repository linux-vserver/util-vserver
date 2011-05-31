// Copyright (C) 2011 Asbjorn Sannes <asbjorn.sannes@interhost.no>
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
vc_get_umask_v23(xid_t xid, struct vc_umask *umask)
{
  int res;
  struct vcmd_umask data;
  res = vserver(VCMD_get_umask, CTX_USER2KERNEL(xid), &data);
  umask->umask = data.umask;
  umask->mask = data.mask;
  return res;
}
