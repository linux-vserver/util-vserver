// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#include <linux/personality.h>

#include <string.h>
#include <strings.h>
#include <assert.h>

#define DECL(VAL) { #VAL, sizeof(#VAL)-1, (PER_ ## VAL) }

static struct Mapping_uint32 const VALUES[] = {
  DECL(LINUX),
  DECL(LINUX_32BIT),
  DECL(SVR4),
  DECL(SVR3),
  DECL(SCOSVR3),
  DECL(OSR5),
  DECL(WYSEV386),
  DECL(ISCR4),
  DECL(BSD),
  DECL(SUNOS),
  DECL(XENIX),
  DECL(LINUX32),
  DECL(LINUX32_3GB),
  DECL(IRIX32),
  DECL(IRIXN32),
  DECL(IRIX64),
  DECL(RISCOS),
  DECL(SOLARIS),
  DECL(UW7),
  DECL(HPUX),
  DECL(OSF4)
};

static char const *
removePrefix(char const *str, size_t *len)
{
  if ((len==0 || *len==0 || *len>4) &&
      strncasecmp("per_", str, 4)==0) {
    if (len && *len>4) *len -= 4;
    return str+4;
  }
  else
    return str;
}

uint_least32_t
vc_str2personalitytype(char const *str, size_t len)
{
  char const	*tmp = removePrefix(str, &len);
  ssize_t	idx  = utilvserver_value2text_uint32(tmp, len,
						     VALUES, DIM_OF(VALUES));

  if (idx==-1) return VC_BAD_PERSONALITY;
  else         return VALUES[idx].val;
}
