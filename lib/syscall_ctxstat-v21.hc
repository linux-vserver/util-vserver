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
vc_ctx_stat_v21(xid_t ctx, struct vc_ctx_stat *stat)
{
  int ret;
  struct vcmd_ctx_stat_v0 param;

  ret = vserver(VCMD_ctx_stat, CTX_USER2KERNEL(ctx), &param);
  if (ret)
    return ret;

  stat->usecnt	= param.usecnt;
  stat->tasks	= param.tasks;
  return 0;
}
