// $Id$    --*- c -*--

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

#include "vserver.h"
#include "pathconfig.h"

#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <assert.h>

vcCfgStyle
vc_getVserverCfgStyle(char const *id)
{
  vcCfgStyle	res = vcCFG_NONE;
  size_t	l1  = strlen(id);
  char		buf[l1 +
		    MAX(sizeof(CONFDIR "/"),sizeof(DEFAULT_VSERVERDIR "/")) +
		    MAX(sizeof("/legacy"),  sizeof(".conf")) - 1];
  char *	marker = 0;

  strcpy(buf,    id);
  marker = buf+l1;
  strcpy(marker, "/vdir");
  
  if (access(buf, X_OK)==0) res = vcCFG_RECENT_FULL;
  else {
    strcpy(buf,                         CONFDIR "/");
    strcpy(buf+sizeof(CONFDIR "/") - 1, id);
    marker = buf+sizeof(CONFDIR "/")+l1 - 1;
    strcpy(marker, "/vdir");

    if (access(buf, X_OK)==0) res = vcCFG_RECENT_SHORT;
    else {
      strcpy(buf,                                  DEFAULT_VSERVERDIR "/");
      strcpy(buf+sizeof(DEFAULT_VSERVERDIR)+1 - 1, id);

      if (access(buf, X_OK)==0) res = vcCFG_LEGACY;
    }

    if (res==vcCFG_LEGACY) {
      strcpy(buf,                            CONFDIR "/");
      strcpy(buf+sizeof(CONFDIR "/") - 1,    id);
      strcpy(buf+sizeof(CONFDIR "/")+l1 - 1, ".conf");

      if (access(buf, R_OK)!=0) res = vcCFG_NONE;
    }
  }


  if (res==vcCFG_RECENT_FULL || res==vcCFG_RECENT_SHORT) {
    assert(marker!=0);
    strcpy(marker, "/legacy");
    if (access(buf, F_OK)==0) res=vcCFG_LEGACY;
  }

  return res;
}
