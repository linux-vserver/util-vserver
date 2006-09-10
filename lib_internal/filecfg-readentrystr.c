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

#include "filecfg.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

char *
FileCfg_readEntryStr (PathInfo const *base, char const *file,
		      bool allow_multiline, char const *dflt)
{
  PathInfo		filepath = { .d = file, .l = strlen(file) };
  PathInfo		path     = *base;
  char			path_buf[ENSC_PI_APPSZ(path, filepath)];
  int			fd  = -1;
  off_t			sz;
  char *		res = 0;

  PathInfo_append(&path, &filepath, path_buf);
  fd = open(path.d, O_RDONLY);
  if (fd==-1) goto err;

  sz = lseek(fd, 0, SEEK_END);
  if (sz==-1 ||
      lseek(fd, 0, SEEK_SET)==-1) goto err;
  
  
  if (sz>0 && sz<FILECFG_MAX_FILESIZE) {
    char		buf[sz+1];
    
    if (read(fd, buf, sz+1)!=sz) goto err;

    if (!allow_multiline) {
      char *		pos;
      
      buf[sz] = '\0';
      pos     = strchr(buf, '\n');
      if (pos) *pos = '\0';
    }
    else {
      while (sz>0 && buf[sz-1]=='\n') --sz;
      buf[sz] = '\0';
    }

    res = strdup(buf);
  }

  err:
  if (res==0 && dflt)
    res = strdup(dflt);
  
  if (fd!=-1) close(fd);
  return res;
}
