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
#include "compat-c99.h"

#include <string.h>
#include <unistd.h>

#ifdef VC_ENABLE_API_COMPAT
#include <dirent.h>
#include <sys/types.h>


static char *
handleLegacy(xid_t xid)
{
  DIR			*dir = opendir(DEFAULT_PKGSTATEDIR);
  struct dirent		*ep;
  char *		result = 0;
  
  if (dir==0) return 0;
  while ((ep=readdir(dir))!=0) {
    char * const		name = ep->d_name;
    size_t			l    = name ? strlen(name) : 0;
    xid_t			cur_xid;

    if (l<=4 || strcmp(name+l-4, ".ctx")!=0) continue;
    name[l-4]   = '\0';
    cur_xid = vc_getVserverCtx(name, vcCFG_LEGACY, false, 0);
    if (cur_xid!=xid) continue;

    result      = strdup(name);
    break;
  }

  closedir(dir);
  return result;
}
#else
static inline char *
handleLegacy(xid_t UNUSED xid)
{
  return 0;
}
#endif

static char *
vc_getVserverByCtx_compat(xid_t ctx, vcCfgStyle *style, char const *revdir,
			  bool validate_result)
{
  if (revdir==0) revdir = DEFAULT_PKGSTATEREVDIR;

  {
  vcCfgStyle	cur_style = vcCFG_NONE;
  size_t	l = strlen(revdir);
  size_t	l1;
  char		path[l + sizeof(unsigned int)*3 + 3];

  strcpy(path, revdir);
  path[l]      = '/';
  l1 = utilvserver_fmt_uint(path+l+1, ctx);
  path[l+1+l1] = '\0';

  if (style==0 || *style==vcCFG_AUTO) {
    if (access(path, F_OK)==0) cur_style = vcCFG_RECENT_FULL;
    else                       cur_style = vcCFG_LEGACY;
  }
  else
    cur_style = *style;

  switch (cur_style) {
    case vcCFG_RECENT_SHORT	:
    case vcCFG_RECENT_FULL	:
	// check if expected ctx == actual ctx (but only when this check is
	// request)
      if (validate_result &&
	  vc_getVserverCtx(path, vcCFG_RECENT_FULL, false, 0)!=ctx) return 0;

      if (style) *style = vcCFG_RECENT_FULL;
      return strdup(path);
	// TODO: handle legacy
    case vcCFG_LEGACY		:
    {
      char *	tmp = handleLegacy(ctx);
      if (tmp && style)
	*style = vcCFG_LEGACY;

      return tmp;
    }
      
    default		:
      return 0;
  }
  }
}
