// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <ensc@delenn.intern.sigma-chemnitz.de>
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
#include "internal.h"
#include <lib_internal/util-dimof.h>
#include <linux/capability.h>

#include <string.h>
#include <strings.h>
#include <assert.h>

#ifndef CAP_QUOTACTL
#  define CAP_QUOTACTL	VC_CAP_QUOTACTL
#endif

#define DECL(VAL) { #VAL, sizeof(#VAL)-1, 1 << (CAP_ ## VAL) }

static struct Mapping_uint64 const VALUES[] = {
  DECL(CHOWN),
  DECL(DAC_OVERRIDE),
  DECL(DAC_READ_SEARCH),
  DECL(FOWNER),
  DECL(FSETID),
  DECL(KILL),
  DECL(SETGID),
  DECL(SETUID),
  DECL(SETPCAP),
  DECL(LINUX_IMMUTABLE),
  DECL(NET_BIND_SERVICE),
  DECL(NET_BROADCAST),
  DECL(NET_ADMIN),
  DECL(NET_RAW),
  DECL(IPC_LOCK),
  DECL(IPC_OWNER),
  DECL(SYS_MODULE),
  DECL(SYS_RAWIO),
  DECL(SYS_CHROOT),
  DECL(SYS_PTRACE),
  DECL(SYS_PACCT),
  DECL(SYS_ADMIN),
  DECL(SYS_BOOT),
  DECL(SYS_NICE),
  DECL(SYS_RESOURCE),
  DECL(SYS_TIME),
  DECL(SYS_TTY_CONFIG),
  DECL(MKNOD),
  DECL(LEASE),
  DECL(QUOTACTL),
};

inline static char const *
removePrefix(char const *str, size_t *len)
{
  if ((len==0 || *len==0 || *len>4) &&
      strncasecmp("cap_", str, 4)==0) {
    if (len && *len>4) *len -= 4;
    return str+4;
  }
  else
    return str;
}

uint_least64_t
vc_text2bcap(char const *str, size_t len)
{
  char const *	tmp = removePrefix(str, &len);
  ssize_t	idx = utilvserver_value2text_uint64(tmp, len,
						    VALUES, DIM_OF(VALUES));
  if (idx==-1) return 0;
  else         return VALUES[idx].val;
}

char const *
vc_lobcap2text(uint_least64_t *val)
{
  ssize_t	idx = utilvserver_text2value_uint64(val,
						    VALUES, DIM_OF(VALUES));

  if (idx==-1) return 0;
  else         return VALUES[idx].id;
}
