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
#include <strings.h>
#include <assert.h>

#define DECL(STR, VAL) { STR, sizeof(STR)-1, VAL }

static struct Mapping_uint64 const VALUES[] = {
  DECL("lock",		VC_NXF_INFO_LOCK),
  DECL("private",	VC_NXF_INFO_PRIVATE),

  DECL("single_ip",	VC_NXF_SINGLE_IP),

  DECL("hide_netif",	VC_NXF_HIDE_NETIF),

  DECL("state_setup",	VC_NXF_STATE_SETUP),
  DECL("state_admin",	VC_NXF_STATE_ADMIN),

  DECL("sc_helper",	VC_NXF_SC_HELPER),
  DECL("persistent",	VC_NXF_PERSISTENT),

    // Aliases for the legacy flags
  DECL("info_lock",	VC_NXF_INFO_LOCK),
  DECL("info_private",	VC_NXF_INFO_PRIVATE),
};

inline static char const *
removePrefix(char const *str, size_t *len)
{
  if ((len==0 || *len==0 || *len>4) &&
      strncasecmp("nxf_", str, 4)==0) {
    if (len && *len>4) *len -= 4;
    return str+4;
  }
  else
    return str;
}

uint_least64_t
vc_text2nflag(char const *str, size_t len)
{
  char const *	tmp = removePrefix(str, &len);
  ssize_t	idx = utilvserver_value2text_uint64(tmp, len,
						    VALUES, DIM_OF(VALUES));
  if (idx==-1) return 0;
  else         return VALUES[idx].val;
}

char const *
vc_lonflag2text(uint_least64_t *val)
{
  ssize_t	idx = utilvserver_text2value_uint64(val,
						    VALUES, DIM_OF(VALUES));

  if (idx==-1) return 0;
  else         return VALUES[idx].id;
}
