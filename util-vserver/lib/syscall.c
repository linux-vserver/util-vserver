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
    res     = vc_get_version(VC_CAT_COMPAT);
    v_errno = errno;
#ifdef VC_ENABLE_API_LEGACY
    if (res==-1 && errno==ENOSYS) res=0;
#endif    
  }

  errno = v_errno;
  return res;
}

#define VC_PREFIX	0)
#define VC_SUFFIX	else (void)((void)0
#define CALL_VC_NOOP	(void)0
#define CALL_VC_GENERAL(ID, SUFFIX, FUNC, ...)				\
  VC_PREFIX; VC_SELECT(ID) return FUNC ## _ ## SUFFIX(__VA_ARGS__); VC_SUFFIX

#if 1
#  define VC_SELECT(ID)	case ID: if(1)
#  define CALL_VC(...)					\
  switch (checkCompatVersion()) {			\
    case -1	:  if (1) break;			\
      VC_SUFFIX, __VA_ARGS__ , VC_PREFIX;		\
    default	:  errno = EINVAL;			\
  }							\
  return -1
#else
#  define VC_SELECT(ID) if (1)
#  define CALL_VC(...)				\
  if (1) {} VC_SUFFIX, __VA_ARGS__, VC_PREFIX;	\
  errno = ENOSYS; return -1
#endif

#ifdef VC_ENABLE_API_COMPAT
#  define CALL_VC_COMPAT(F,...) CALL_VC_GENERAL(0x00010000, compat, F, __VA_ARGS__)
#else
#  define CALL_VC_COMPAT(F,...)	CALL_VC_NOOP
#endif

#ifdef VC_ENABLE_API_LEGACY
#  define CALL_VC_LEGACY(F,...) CALL_VC_GENERAL(0x00000000, legacy, F, __VA_ARGS__)
#else
#  define CALL_VC_LEGACY(F,...) CALL_VC_NOOP
#endif

int
vc_get_version(int cat)
{
  return sys_virtual_context(VC_CMD(VERSION, 0, 0), cat, 0);
}

#if defined(VC_ENABLE_API_COMPAT) || defined(VC_ENABLE_API_LEGACY)

int
vc_new_s_context(ctx_t ctx, unsigned int remove_cap, unsigned int flags)
{
  CALL_VC(CALL_VC_COMPAT(vc_new_s_context, ctx, remove_cap, flags),
	  CALL_VC_LEGACY(vc_new_s_context, ctx, remove_cap, flags));
}

int
vc_set_ipv4root(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips)
{
  CALL_VC(CALL_VC_COMPAT(vc_set_ipv4root, bcast, nb, ips),
	  CALL_VC_LEGACY(vc_set_ipv4root, bcast, nb, ips));
}

int
vc_chrootsafe(char const *dir)
{
  CALL_VC(CALL_VC_COMPAT(vc_chrootsafe, dir),
	  CALL_VC_LEGACY(vc_chrootsafe, dir));
}

#endif
