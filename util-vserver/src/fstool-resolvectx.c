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

#include "util.h"
#include <lib/vserver.h>

#include <stdlib.h>

xid_t
resolveCtx(char const *str)
{
  xid_t		res;
  
  if (*str==':') ++str;
  else {
    char	*end_ptr;
    long	result = strtol(str, &end_ptr, 0);

    if (end_ptr>str && *end_ptr==0) return result;
  }

  res = vc_getVserverCtx(str, vcCFG_AUTO, true, 0);
  if (res==VC_NOCTX) {
    WRITE_MSG(2, "Can not find a vserver with name '");
    WRITE_STR(2, str);
    WRITE_MSG(2, "', or vserver does not have a static context\n");
    exit(1);
  }

  return res;
}
