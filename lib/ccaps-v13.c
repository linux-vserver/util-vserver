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

#include <string.h>
#include <assert.h>

#define DECL(STR, VAL) { STR, sizeof(STR)-1, VAL }

static struct Mapping_uint64 const VALUES[] = {
  DECL("set_utsname",     VC_VXC_SET_UTSNAME),
  DECL("rlimit",          VC_VXC_SET_RLIMIT),
  DECL("raw_icmp",        VC_VXC_RAW_ICMP),
  DECL("syslog",          VC_VXC_SYSLOG),
  DECL("secure_mount",    VC_VXC_SECURE_MOUNT),
  DECL("secure_remount",  VC_VXC_SECURE_REMOUNT),
  DECL("binary_mount",    VC_VXC_BINARY_MOUNT),
  DECL("quota_ctl",       VC_VXC_QUOTA_CTL),
  DECL("admin_mapper",    VC_VXC_ADMIN_MAPPER),
  DECL("admin_cloop",     VC_VXC_ADMIN_CLOOP),
    // some deprecated values...
  DECL("mount",           VC_VXC_SECURE_MOUNT),
  DECL("remount",         VC_VXC_SECURE_REMOUNT),
  DECL("icmp",            VC_VXC_RAW_ICMP),
  DECL("ping",            VC_VXC_RAW_ICMP),
  DECL("utsname",         VC_VXC_SET_UTSNAME),
};

uint_least64_t
vc_text2ccap(char const *str, size_t len)
{
  ssize_t	idx = utilvserver_value2text_uint64(str, len,
						    VALUES, DIM_OF(VALUES));
  if (idx==-1) return 0;
  else         return VALUES[idx].val;
}

char const *
vc_loccap2text(uint_least64_t *val)
{
  ssize_t	idx = utilvserver_text2value_uint64(val,
						    VALUES, DIM_OF(VALUES));

  if (idx==-1) return 0;
  else         return VALUES[idx].id;
}
