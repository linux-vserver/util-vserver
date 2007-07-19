// $Id$    --*- c -*--

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
vc_get_sched_v21(xid_t xid, struct vc_set_sched *data)
{
  struct vcmd_sched_v5 k_data = {
    .mask	= data->set_mask,
    .cpu_id	= data->cpu_id,
    .bucket_id	= data->bucket_id,
  };
  int ret;

  ret = vserver(VCMD_get_sched, xid, &k_data);
  data->set_mask      = k_data.mask;
  data->cpu_id        = k_data.cpu_id;
  data->bucket_id     = k_data.bucket_id;
  data->fill_rate     = k_data.fill_rate[0];
  data->fill_rate2    = k_data.fill_rate[1];
  data->interval      = k_data.interval[0];
  data->interval2     = k_data.interval[1];
  data->tokens        = k_data.tokens;
  data->tokens_min    = k_data.tokens_min;
  data->tokens_max    = k_data.tokens_max;
  data->priority_bias = k_data.prio_bias;

  return ret;
}
