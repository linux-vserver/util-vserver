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

nid_t
vc_nidopt2nid(char const *str, bool honor_static, char const **err_info)
{
  char const *		err;
  nid_t			res = VC_NOCTX;

  err = "vc_get_task_nid()";
  if (strcmp(str,"self")==0) res = vc_get_task_nid(0);
  else                       res = vc_xidopt2xid(str, honor_static, &err);

  if (res==VC_NOCTX && err_info) *err_info = err;

  return res;
}
