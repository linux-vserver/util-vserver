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

#include "vserver.h"
#include "vserver-internal.h"
#include "linuxvirtual.h"

#ifdef VC_ENABLE_API_COMPAT    
#  include "syscall-compat.hc"
#endif

#ifdef VC_ENABLE_API_LEGACY
#  include "syscall-legacy.hc"
#endif

#include <stdbool.h>
#include <errno.h>



static int
checkCompatVersion()
{
  static int	res=0;
  static int	v_errno;

  if (res==0) {
    res     = sys_virtual_context(VC_CMD(VERSION, 0, 0), VC_CAT_COMPAT, 0);
    v_errno = errno;
#ifdef VC_ENABLE_API_LEGACY
    if (res==-1 && errno==ENOSYS) res=0;
#endif    
  }

  errno = v_errno;
  return res;
}

int
vc_new_s_context(ctx_t ctx, unsigned int remove_cap, unsigned int flags)
{
  switch (checkCompatVersion()) {
#ifdef VC_ENABLE_API_COMPAT    
    case 0x00010000	:
      return vc_new_s_context_compat(ctx, remove_cap, flags);
#endif      
#ifdef VC_ENABLE_API_LEGACY
    case 0x0000000	:
      return vc_new_s_context_legacy(ctx, remove_cap, flags);
#endif
    case -1		:  break;
    default		:  errno = EINVAL;
  }

  return -1;
}

int
vc_set_ipv4root(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips)
{
  switch (checkCompatVersion()) {
#ifdef VC_ENABLE_API_COMPAT    
    case 0x00010000	:
      return vc_set_ipv4root_compat(bcast, nb, ips);
#endif
#ifdef VC_ENABLE_API_LEGACY
    case 0x0000000	:
      return vc_set_ipv4root_legacy(bcast, nb, ips);
#endif
    case -1		:  break;
    default		:  errno = EINVAL;
  }

  return -1;
}

int
vc_chrootsafe(char const *dir)
{
  switch (checkCompatVersion()) {
#ifdef VC_ENABLE_API_COMPAT    
    case 0x00010000	:
      return vc_chrootsafe_compat(dir);
#endif
#ifdef VC_ENABLE_API_LEGACY
    case 0x0000000	:
      return vc_chrootsafe_legacy(dir);
#endif
    case -1		:  break;
    default		:  errno = EINVAL;
  }

  return -1;
}
