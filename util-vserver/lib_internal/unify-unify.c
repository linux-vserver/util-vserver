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

#include "unify.h"
#include "vserver.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

bool
Unify_unify(char const *src, struct stat const UNUSED *src_stat,
	    char const *dst)
{
  size_t	l = strlen(dst);
  char		tmpfile[l + sizeof(";XXXXXX")];
  int		fd;
  bool		res = false;

  // at first, set the ILI flags on 'src'
  if (vc_set_iattr(src,
		   0,
		   VC_IATTR_IUNLINK|VC_IATTR_IMMUTABLE,
		   VC_IATTR_IUNLINK|VC_IATTR_IMMUTABLE)==-1)
    return false;

  // now, create a temporary filename
  memcpy(tmpfile,   dst, l);
  memcpy(tmpfile+l, ";XXXXXX", 8);
  fd = mkstemp(tmpfile);
  close(fd);

  if (fd==-1) {
    perror("mkstemp()");
    return false;
  }

  // and rename the old file to this name

  // NOTE: this rename() is race-free; when an attacker makes 'tmpfile' a
  // directory, the operation would fail; when making it a symlink to a file
  // or directory, the symlink but not the file/directory would be overridden
  if (rename(dst, tmpfile)==-1) {
    perror("rename()");
    goto err;
  }

  // now, link the src-file to dst
  if (link(src, dst)==-1) {
    perror("link()");

    unlink(dst);
    if (rename(tmpfile, dst)==-1) {
      perror("FATAL error in rename()");
      _exit(1);
    }
    goto err;
  }

  res = true;

  err:
  unlink(tmpfile);

  return res;
}
