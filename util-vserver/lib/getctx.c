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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "compat.h"
#include "vserver.h"
#include "internal.h"
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define CTX_TAG		"\ns_context: "

ctx_t
getctx(pid_t pid)
{
  static volatile size_t	bufsize=4097;
    // TODO: is this really race-free?
  size_t			cur_bufsize = bufsize;
  int				fd;
  char				status_name[ sizeof("/proc/01234/status") ];
  char				buf[cur_bufsize];
  size_t			len;
  char				*pos = 0;

  strcpy(status_name, "/proc/");
  len = utilvserver_uint2str(status_name+sizeof("/proc/")-1,
			     sizeof(status_name)-sizeof("/proc//status")+1,
			     pid, 10);
  strcpy(status_name+sizeof("/proc/")+len-1, "/status");

  fd = open(status_name, O_RDONLY);
  if (fd==-1) return CTX_NOCTX;

  len = read(fd, buf, cur_bufsize);
  close(fd);

  if (len<cur_bufsize)
    pos      = strstr(buf, CTX_TAG);
  else if (len!=(size_t)-1) {
    bufsize  = cur_bufsize * 2 - 1;
    errno    = EAGAIN;
  }

  if (pos!=0) return atoi(pos+sizeof(CTX_TAG)-1);
  else        return CTX_NOCTX;
}

#if 0
ctx_t
getcctx()
{
  return getctx(getpid());
}
#endif
