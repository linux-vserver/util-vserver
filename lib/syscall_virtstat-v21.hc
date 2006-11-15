// $Id$    --*- c++ -*--

// Copyright (C) 2006 Daniel Hokka Zakrisson <daniel@hozac.com>
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
vc_virt_stat_v21(xid_t ctx, struct vc_virt_stat *stat)
{
  int ret;
  struct vcmd_virt_stat_v0 param;

  ret = vserver(VCMD_virt_stat, CTX_USER2KERNEL(ctx), &param);
  if (ret)
    return ret;

#define G(ATTR)	stat->ATTR = param.ATTR
  G(offset);
  G(uptime);
  G(nr_threads);
  G(nr_running);
  G(nr_uninterruptible);
  G(nr_onhold);
  G(nr_forks);
  G(load[0]);
  G(load[1]);
  G(load[2]);
  return 0;
}
