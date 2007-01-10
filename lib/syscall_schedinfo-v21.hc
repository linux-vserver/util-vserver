// $Id$    --*- c++ -*--

// Copyright (C) 2007 Daniel Hokka Zakrisson <daniel@hozac.com>
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
vc_sched_info_v21(xid_t ctx, struct vc_sched_info *info)
{
  int ret;
  struct vcmd_sched_info param = { .cpu_id = info->cpu_id, .bucket_id = info->bucket_id };

  ret = vserver(VCMD_sched_info, CTX_USER2KERNEL(ctx), &param);
  if (ret)
    return ret;

#define G(ATTR)	info->ATTR = param.ATTR
  G(user_msec);
  G(sys_msec);
  G(hold_msec);
  G(token_usec);
  G(vavavoom);

  return 0;
}
