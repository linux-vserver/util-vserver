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
vc_net_add_net(nid_t nid, struct vc_net_addr const *info)
{
  struct vcmd_net_addr_v0		k_info;
  size_t				i;

  k_info.type             = info->vna_type & (VC_NXA_TYPE_IPV4|VC_NXA_TYPE_IPV6);
  k_info.count            = 1;
  switch (info->vna_type & ~VC_NXA_TYPE_ADDR) {
    case VC_NXA_TYPE_IPV4:
      k_info.ip[0].s_addr   = info->vna_v4_ip.s_addr;
      k_info.mask[0].s_addr = info->vna_v4_mask.s_addr;
      break;
    case VC_NXA_TYPE_IPV6:
      for (i = 0; i < 4; i++)
	k_info.ip[i].s_addr = info->vna_v6_ip.s6_addr32[i];
      k_info.mask[0].s_addr = info->vna_prefix;
      break;
    default:
      errno = EINVAL;
      return -1;
  }

  return vserver(VCMD_net_add_v0, NID_USER2KERNEL(nid), &k_info);
}
