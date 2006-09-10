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
#include "pathconfig.h"

#include "matchlist.h"
#include "util-io.h"

#include "vserver.h"

#include <assert.h>

bool
MatchList_initByVserver(struct MatchList *list,
			struct MatchVserverInfo const *vserver)
{
  assert(vserver->appdir!=0 && vserver->vdir!=0);
  
  size_t const	l = vserver->appdir.l;
  char		tmp[l + sizeof("/exclude")];
  char const *	excl_list;

  memcpy(tmp,   vserver->appdir.d, l);
  memcpy(tmp+l, "/exclude", 9);

  excl_list = tmp;
  if (access(excl_list, R_OK)==-1) excl_list = CONFDIR   "/.defaults/apps/vunify/exclude";
  if (access(excl_list, R_OK)==-1) excl_list = PKGLIBDIR "/defaults/vunify-exclude";

  MatchList_initManually(list, vserver, 0, excl_list);
  
  return true;
}

