// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#include "compat.h"

#include "safechroot-internal.hc"

#include "vserver.h"
#include "vserver-internal.h"

#include <unistd.h>

static inline ALWAYSINLINE int
vc_new_s_context_compat(ctx_t ctx, unsigned int remove_cap, unsigned int flags)
{
  struct vcmd_new_s_context_v1	msg;
  msg.remove_cap = remove_cap;
  msg.flags      = flags;

  return sys_vserver(VC_CMD(COMPAT, 1, 1), CTX_USER2KERNEL(ctx), &msg);
}

static inline ALWAYSINLINE int
vc_set_ipv4root_compat(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips)
{
  struct vcmd_set_ipv4root_v3	msg;
  size_t			i;

  if (nb>=NB_IPV4ROOT) {
    errno = -EINVAL;
    return -1;
  }

  msg.broadcast = bcast;

  for (i=0; i<nb; ++i) {
    msg.ip_mask_pair[i].ip   = ips[i].ip;
    msg.ip_mask_pair[i].mask = ips[i].mask;
  }

  return sys_vserver(VC_CMD(COMPAT, 2, 3), nb, &msg);
}

static inline ALWAYSINLINE int
vc_chrootsafe_compat(char const *dir)
{
  vc_tell_unsafe_chroot();
  return chroot(dir);
}
