// $Id$    --*- c++ -*--

// Copyright (C) 2007 Daniel Hokka Zakrisson
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
vc_get_badness_v23(xid_t xid, int64_t *badness)
{
  struct vcmd_badness_v0 data;
  int ret;
  ret = vserver(VCMD_get_badness, xid, &data);
  if (ret)
    return ret;
  *badness = data.bias;
  return 0;
}
