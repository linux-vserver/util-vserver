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
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#include <limits.h>

static char *
getRecentName(char *start, char *end)
{
  char		*res = 0;
  int		fd;
  char		buf[PATH_MAX];
  
  strcpy(end, "/name");
  fd = open(start, O_RDONLY);
  if (fd!=-1) {
    off_t	len;

    if ((len=lseek(fd, 0, SEEK_END))!=-1 &&
	(len<VC_LIMIT_VSERVER_NAME_LEN) &&
	(lseek(fd, 0, SEEK_SET)!=-1)) {
      char	buf[len+1];

      if (TEMP_FAILURE_RETRY(read(fd, buf, len+1))==len) {
	while (len>0 && buf[len-1]=='\n') --len;
	buf[len] = '\0';
	if (len>0) res = buf;
      }

      close(fd);
      return strdup(res);
    }

    close(fd);
  }

  if (res==0) {
    *end = '\0';
    res  = realpath(start, buf);
    //printf("start='%s', res='%s'\n", start,res);
    if (res==0) res = start;

    res = basename(res);
  }

  return strdup(res);
}

char *
vc_getVserverName(char const *id, vcCfgStyle style)
{
  size_t		l1  = strlen(id);

  if (style==vcCFG_NONE || style==vcCFG_AUTO)
    style = vc_getVserverCfgStyle(id);

  switch (style) {
    case vcCFG_NONE		:  return 0;
    case vcCFG_LEGACY		:  return strdup(id);
    case vcCFG_RECENT_SHORT	:
    {
      char		buf[sizeof(CONFDIR "/") + l1 + sizeof("/name") - 1];

      strcpy(buf,                         CONFDIR "/");
      strcpy(buf+sizeof(CONFDIR "/") - 1, id);
      
      return getRecentName(buf, buf+sizeof(CONFDIR "/")+l1 - 1);
    }
    case vcCFG_RECENT_FULL	:
    {
      char		buf[l1 + sizeof("/name")];
      strcpy(buf, id);

      return getRecentName(buf, buf+l1);
    }
    default			:  return 0;
  }
}
