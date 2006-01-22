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
#include <lib_internal/util.h>

#define X(ATTR)		ENSC_SAME_STRUCT_IDX(k_data, *data, ATTR)

static inline ALWAYSINLINE int
vc_set_sched_v13b(xid_t xid, struct vc_set_sched const *data)
{
  struct vcmd_set_sched_v3	k_data;

    // This expression will be evaluated at compile-time
  if (sizeof(struct vcmd_set_sched_v3)==sizeof(struct vc_set_sched) &&
      X(set_mask)   && X(fill_rate)  && X(interval)   && X(tokens) &&
      X(tokens_min) && X(tokens_max) && X(priority_bias))
    return vserver(VCMD_set_sched, CTX_USER2KERNEL(xid),
		   const_cast(struct vc_set_sched *)(data));
  else {
    k_data.set_mask      = data->set_mask;
    k_data.fill_rate     = data->fill_rate;
    k_data.interval      = data->interval;
    k_data.tokens        = data->tokens;
    k_data.tokens_min    = data->tokens_min;
    k_data.tokens_max	 = data->tokens_max;
    k_data.priority_bias = data->priority_bias;

    return vserver(VCMD_set_sched, CTX_USER2KERNEL(xid), &k_data);
  }
}
