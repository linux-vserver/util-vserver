// $Id$    --*- c++ -*--

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


#ifndef H_UTIL_VSERVER_LIB_GETINITPID_LEGACY_H
#define H_UTIL_VSERVER_LIB_GETINITPID_LEGACY_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include "vserver.h"
#include "utils-legacy.h"

#include <string.h>
#include <errno.h>
#include <sys/types.h>

static pid_t
vc_X_getinitpid_legacy_internal(pid_t pid)
{
  size_t			bufsize = utilvserver_getProcEntryBufsize();
  char				buf[bufsize];
  char				*pos = 0;

  pos = utilvserver_getProcEntry(pid, "\ninitpid: ", buf, bufsize);

  if (pos!=0) {
    if (memcmp(pos,"none",4)==0) return 1;
    else                         return atoi(pos);
  }
  else        return -1;
}

static pid_t
vc_X_getinitpid_legacy(pid_t pid)
{
  pid_t		res;
  do {
    res = vc_X_getinitpid_legacy_internal(pid);
  } while (res==-1 && errno==EAGAIN);

  return res;
}

#endif	//  H_UTIL_VSERVER_LIB_GETINITPID_LEGACY_H
