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

// Saves current ctx + vserver-info into 'argv[1] + /run' which must be a dead
// symlink

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include "vserver.h"
#include "internal.h"
#include "util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>

inline static void
checkParams(int argc, char UNUSED * argv[])
{
  if (argc<3) {
    WRITE_MSG(2, "Usage:  save_ctxinfo <VSERVER_DIR> <cmd> <args>*\n");
    exit(255);
  }
}

int main(int argc, char *argv[])
{
  char		runfile[(checkParams(argc,argv),strlen(argv[1])) + sizeof("/run")];
  char		dstfile[PATH_MAX];
  int		fd;
  char		buf[32];
  ctx_t		ctx;
  ssize_t	len;
  ssize_t	len1 = strlen(argv[1]);

  strcpy(runfile,      argv[1]);
  strcpy(runfile+len1, "/run");

  ctx=vc_X_getctx(0);
  if (ctx==-1) {
    perror("vc_X_getcctx()");
    return -1;
  }

  if (readlink(runfile, dstfile, sizeof(dstfile))==-1) {
    perror("readlink()");
    return -1;
  }

  fd = open(dstfile, O_EXCL|O_CREAT|O_WRONLY, 0644);
  if (fd==-1) {
    perror("open()");
    return -1;
  }

  len  = utilvserver_uint2str(buf, sizeof(buf), ctx, 10);

  if (write(fd, buf,     len) !=len  ||
      write(fd, "\n",    1)   !=1    ||
      write(fd, argv[1], len1)!=len1 ||
      write(fd, "\n",    1)   !=1) {
    perror("write()");
    return -1;
  }

  if (close(fd)==-1) {
    perror("close()");
    return -1;
  }

  execv(argv[2], argv+2);
  perror("execv()");
  return -1;
}
