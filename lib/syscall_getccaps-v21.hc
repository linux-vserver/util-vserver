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

static inline ALWAYSINLINE int
vc_get_ccaps_v21(xid_t xid, struct vc_ctx_caps *caps)
{
  struct vcmd_ctx_caps_v1	k_ccaps;
  struct vcmd_bcaps		k_bcaps;
  int				res;
  
  res = vserver(VCMD_get_ccaps, CTX_USER2KERNEL(xid), &k_ccaps);
  if (res)
    return res;
  res = vserver(VCMD_get_bcaps, CTX_USER2KERNEL(xid), &k_bcaps);
  if (res)
    return res;

  caps->bcaps = k_bcaps.bcaps;
  caps->bmask = k_bcaps.bmask;
  caps->ccaps = k_ccaps.ccaps;
  caps->cmask = k_ccaps.cmask;

  return res;
}
