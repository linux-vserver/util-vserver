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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

static ctx_t
getCtxFromFile(char const *pathname)
{
  int		fd;
  off_t		len;

  fd = open(pathname, O_RDONLY);

  if (fd==-1 ||
      (len=lseek(fd, 0, SEEK_END))==-1 ||
      (len>50) ||
      (lseek(fd, 0, SEEK_SET)==-1))
    return VC_NOCTX;
  
  char	buf[len+1];
  if (TEMP_FAILURE_RETRY(read(fd, buf, len+1))!=len)
    return VC_NOCTX;

  char		*errptr;
  ctx_t		res;

  res = strtol(buf, &errptr, 10);
  if (*errptr!='\0' && *errptr!='\n') return VC_NOCTX;

  return res;
}

ctx_t
vc_getVserverCtx(char const *id, vcCfgStyle style)
{
  size_t		l1 = strlen(id);
  char			buf[sizeof(CONFDIR "//") + l1 + sizeof("/run")];
			    
  if (style==vcCFG_NONE || style==vcCFG_AUTO)
    style = vc_getVserverCfgStyle(id);
  
  switch (style) {
    case vcCFG_NONE		:  return VC_NOCTX;
    case vcCFG_LEGACY		:  return VC_NOCTX;	// todo
    case vcCFG_RECENT_SHORT	:
    case vcCFG_RECENT_FULL	: {
      size_t		idx = 0;
      if (style==vcCFG_RECENT_SHORT) {
	memcpy(buf, CONFDIR "/", sizeof(CONFDIR "/")-1);
	idx  = sizeof(CONFDIR "/") - 1;
      }
      memcpy(buf+idx, id, l1);    idx += l1;
      memcpy(buf+idx, "/run", 4); idx += 4;
      buf[idx] = '\0';
      

      return getCtxFromFile(buf);
    }
    default			:  return VC_NOCTX;
  }
}
