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

#include "utils-legacy.h"
#include "internal.h"
#include "vserver-internal.h"

#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

static volatile size_t		proc_bufsize = 4097;

size_t
utilvserver_getProcEntryBufsize()
{
  return proc_bufsize;
}

char *
utilvserver_getProcEntry(pid_t pid,
			 char *str,
			 char *buf, size_t bufsize)
{
  char			status_name[ sizeof("/proc/01234/status") ];
  int			fd;
  size_t		len;
  char *		res = 0;

  if (pid<0 || (uint32_t)(pid)>99999) {
    errno = EINVAL;
    return 0;
  }

  if (pid==0) strcpy(status_name, "/proc/self/status");
  else {
    strcpy(status_name, "/proc/");
    len = utilvserver_uint2str(status_name+sizeof("/proc/")-1,
			       sizeof(status_name)-sizeof("/proc//status")+1,
			       pid, 10);
    strcpy(status_name+sizeof("/proc/")+len-1, "/status");
  }

  fd = open(status_name, O_RDONLY);
  if (fd==-1) return 0;

  len = read(fd, buf, bufsize);
  close(fd);

  if (len<bufsize) {
    buf[len] = '\0';
    if (str)
      res    = strstr(buf, str) + strlen(str);
    else
      res    = buf;
  }
  else if (len!=(size_t)-1) {
    if (proc_bufsize==bufsize)
      proc_bufsize = bufsize * 2 - 1;

    errno = EAGAIN;
  }

  return res;
}
