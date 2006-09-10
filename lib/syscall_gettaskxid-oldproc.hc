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

#include "utils-legacy.h"

static xid_t
getctx_legacy_internal(pid_t pid)
{
  size_t			bufsize = utilvserver_getProcEntryBufsize();
  char				buf[bufsize];
  char				*pos = 0;

  pos = utilvserver_getProcEntry(pid, "\ns_context: ", buf, bufsize);

  if (pos!=0) return atoi(pos);
  else        return VC_NOCTX;
}

static inline ALWAYSINLINE xid_t
vc_get_task_xid_oldproc(pid_t pid)
{
  xid_t		res;
  do {
    res = getctx_legacy_internal(pid);
  } while (res==VC_NOCTX && errno==EAGAIN);

  return res;
}
