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

#include "vserver.h"
#include <string.h>
#include <stdlib.h>

static xid_t
getVserverXid(char const *id, bool honor_static, char const **err)
{
  *err = "vc_getVserverCtx";
  return vc_getVserverCtx(id, vcCFG_AUTO, honor_static, 0);
}

xid_t
vc_xidopt2xid(char const *str, bool honor_static, char const **err_info)
{
  char const *		err;
  xid_t			res = VC_NOCTX;

  err = "vc_get_task_xid()";
  if (strcmp(str,"self")==0) res = vc_get_task_xid(0);
  else if (str[0]==':')      res = getVserverXid(str+1, honor_static, &err);
  else {
    char *	endptr;
    xid_t	xid = strtol(str, &endptr, 10);

    if (endptr!=str && (*endptr=='\0' || *endptr=='\n'))
      res = xid;
    else
      res = getVserverXid(str, honor_static, &err);
  }

  if (res==VC_NOCTX && err_info) *err_info = err;

  return res;
}
