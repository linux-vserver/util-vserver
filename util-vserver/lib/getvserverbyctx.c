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

char *
vc_getVserverByCtx(ctx_t ctx, vcCfgStyle *style, char const *revdir)
{
  if (revdir==0) revdir = DEFAULT_PKGSTATEREVDIR;

  BS;
  vcCfgStyle	cur_style = vcCFG_NONE;
  size_t	l = strlen(revdir);
  size_t	l1;
  char		path[l + sizeof(unsigned int)*3 + 2 + sizeof("/name")];

  strcpy(path, revdir);
  path[l]      = '/';
  l1 = utilvserver_fmt_uint(path+l+1, ctx);
  path[l+1+l1] = '\0';

  if (style==0 || *style==vcCFG_AUTO) {
    if (access(path, F_OK)==0) cur_style = vcCFG_RECENT_FULL;
      // TODO: handle legacy
  }
  else
    cur_style = *style;

  switch (cur_style) {
    case vcCFG_RECENT_SHORT	:
    case vcCFG_RECENT_FULL	:
	// check if expected ctx == actual ctx
      if (vc_getVserverCtx(path, vcCFG_RECENT_FULL)!=ctx) return 0;

      if (style) *style = vcCFG_RECENT_FULL;
      return strdup(path);
	// TODO: handle legacy
    default		:
      return 0;
  }
  BE;
}
