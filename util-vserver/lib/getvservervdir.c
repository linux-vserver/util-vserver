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
#include "internal.h"
#include "pathconfig.h"

#include <string.h>

char *
vc_getVserverVdir(char const *id, vcCfgStyle style)
{
  size_t		l1   = strlen(id);
  char			*res = 0;

  if (style==vcCFG_NONE || style==vcCFG_AUTO)
    style = vc_getVserverCfgStyle(id);

  switch (style) {
    case vcCFG_NONE		:  return 0;
    case vcCFG_LEGACY		:
    {
      char		buf[sizeof(DEFAULT_VSERVERDIR "/") + l1];

      strcpy(buf,                                    DEFAULT_VSERVERDIR "/");
      strcpy(buf+sizeof(DEFAULT_VSERVERDIR "/") - 1, id);

      res = strdup(buf);
      break;
    }
    
    case vcCFG_RECENT_SHORT	:
    {
      char		buf[sizeof(CONFDIR) + l1 + sizeof("//vdir") - 1];

      strcpy(buf,                            CONFDIR "/");
      strcpy(buf+sizeof(CONFDIR "/")    - 1, id);
      strcpy(buf+sizeof(CONFDIR "/")+l1 - 1, "/vdir");
      
      res = strdup(buf);
      break;
    }

    case vcCFG_RECENT_FULL	:
    {
      char		buf[l1 + sizeof("/vdir")];

      strcpy(buf,    id);
      strcpy(buf+l1, "/vdir");

      res = strdup(buf);
      break;
    }

    default			:  return 0;
  }

  if (!utilvserver_isDirectory(res, true)) {
    free(res);
    res = 0;
  }

  return res;
}
