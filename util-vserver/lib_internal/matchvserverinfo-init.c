// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "matchlist.h"
#include "util.h"

#include <pathconfig.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/param.h>

static bool
exists(char *path, size_t len, char const *name)
{
  if (name) strcpy(path + len, name);

  return access(path, R_OK)!=-1;
}

#define EXISTS(X)		exists(X, sizeof(X)-1, 0)
#define DEFAULT_EXISTS(X)	EXISTS(CONFDIR "/.defaults/apps/vunify/" X)

bool
MatchVserverInfo_init(struct MatchVserverInfo *info)
{
  assert(info->name!=0);
  assert(info->vdir.d==0 && info->appdir.d==0);
  
  info->style    = vc_getVserverCfgStyle(info->name);
  info->vdir.d   = vc_getVserverVdir    (info->name, info->style, true);
  info->appdir.d = vc_getVserverAppDir  (info->name, info->style, "vunify");

  if (info->vdir.d==0 || info->appdir.d==0) {
    free(const_cast(char *)(info->vdir.d));
    free(const_cast(char *)(info->appdir.d));

    return false;
  }

  info->vdir.l   = strlen(info->vdir.d);
  info->appdir.l = strlen(info->appdir.d);

  size_t const	l = info->appdir.l;
  char		tmp[l + MAX(sizeof("/pkgmgmt-ignore"),sizeof("/pkgmgmt-force"))];

  memcpy(tmp,    info->appdir.d, l);

  if      (exists(tmp, l, "/pkgmgmt-ignore")) info->use_pkgmgmt = false;
  else if (exists(tmp, l, "/pkgmgmt-force"))  info->use_pkgmgmt = true;
  else if (DEFAULT_EXISTS("pkgmgmt-ignore"))  info->use_pkgmgmt = false;
  else if (DEFAULT_EXISTS("pkgmgmt-force"))   info->use_pkgmgmt = true;

  return true;
}
