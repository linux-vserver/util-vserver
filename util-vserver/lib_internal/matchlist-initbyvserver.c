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



bool
MatchList_initByVserver(struct MatchList *list, char const *vserver,
			char const **res_appdir)
{
  vcCfgStyle	style;
  char const	*vdir;
  char const 	*appdir;

  style  = vc_getVserverCfgStyle(vserver);
  vdir   = vc_getVserverVdir(  vserver, style, true);
  appdir = vc_getVserverAppDir(vserver, style, "vunify");

  if (vdir==0 || appdir==0) {
    free((char *)appdir);
    free((char *)vdir);
    return false;
  }

  {
    size_t		l1 = strlen(appdir);
    char		tmp[l1 + sizeof("/exclude")];
    char const *	excl_list;

    memcpy(tmp,    appdir, l1);
    memcpy(tmp+l1, "/exclude", 9);

    excl_list = tmp;
    if (access(excl_list, R_OK)==-1) excl_list = CONFDIR   "/.defaults/apps/vunify/exclude";
    if (access(excl_list, R_OK)==-1) excl_list = PKGLIBDIR "/defaults/vunify-exclude";

      // 'vdir' is transferred to matchlist and must not be free'ed here
    MatchList_initManually(list, vserver, vdir, excl_list);
  }

  if (res_appdir!=0)
    *res_appdir = appdir;
  else
    free((char *)appdir);
  
  return true;
}

