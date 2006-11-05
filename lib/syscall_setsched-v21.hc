// $Id$    --*- c -*--

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

#include "vserver.h"

static inline ALWAYSINLINE int
vc_set_sched_v21(xid_t xid, struct vc_set_sched const *data)
{
  struct vcmd_set_sched_v4	k_data;
  int ret;

  k_data.fill_rate  = data->fill_rate;
  k_data.interval   = data->interval;
  k_data.tokens     = data->tokens;
  k_data.tokens_min = data->tokens_min;
  k_data.tokens_max = data->tokens_max;
  k_data.prio_bias  = data->priority_bias;
  k_data.cpu_id	    = data->cpu_id;
  k_data.bucket_id  = data->bucket_id;

  if ((data->set_mask & VC_VXSM_FILL_RATE2 && data->fill_rate != data->fill_rate2) ||
      (data->set_mask & VC_VXSM_INTERVAL2 && data->interval != data->interval2)) {
    /* first set the regular values */
    k_data.set_mask = data->set_mask & ~(VC_VXSM_FILL_RATE2|VC_VXSM_INTERVAL2|VC_VXSM_FORCE);
    ret = vserver(VCMD_set_sched, CTX_USER2KERNEL(xid), &k_data);
    if (ret)
      return ret;
    /* then set idle time */
    k_data.set_mask  = data->set_mask & (VC_VXSM_FILL_RATE2|VC_VXSM_INTERVAL2|VC_VXSM_FORCE|VC_VXSM_IDLE_TIME|VC_VXSM_CPU_ID|VC_VXSM_BUCKET_ID);
    k_data.fill_rate = data->fill_rate2;
    k_data.interval  = data->interval2;
    return vserver(VCMD_set_sched, CTX_USER2KERNEL(xid), &k_data);
  }
  else {
    k_data.set_mask = data->set_mask;
    return vserver(VCMD_set_sched, CTX_USER2KERNEL(xid), &k_data);
  }
}
