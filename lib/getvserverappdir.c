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
#include "internal.h"
#include "pathconfig.h"

#include <string.h>

char *
vc_getVserverAppDir(char const *id, vcCfgStyle style, char const *app)
{
  size_t		l1   = strlen(id);
  size_t		l2   = strlen(app);
  char			*res = 0;

  if (style==vcCFG_NONE || style==vcCFG_AUTO)
    style = vc_getVserverCfgStyle(id);

  switch (style) {
    case vcCFG_NONE		:  return 0;
    case vcCFG_LEGACY		:  return 0;
    case vcCFG_RECENT_FULL	:
    case vcCFG_RECENT_SHORT	:
    {
      char		buf[sizeof(CONFDIR) + l1 + l2 + sizeof("//apps/") - 1];
      char *		ptr = buf;

      if (style==vcCFG_RECENT_FULL)
	memcpy(ptr, id, l1);
      else {
	memcpy(ptr, CONFDIR "/", sizeof(CONFDIR "/")-1);
	ptr += sizeof(CONFDIR "/")-1;
	memcpy(ptr, id, l1);      
      }
      
      ptr += l1;
      memcpy(ptr, "/apps/", 6); ptr += 6;
      memcpy(ptr, app,     l2); ptr += l2;
      *ptr = '\0';
      
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
