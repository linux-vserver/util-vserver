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

#include "pathconfig.h"
#include "vserver.h"
#include "internal.h"
#include "util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

#define ENSC_WRAPPERS_PREFIX	"save_ctxinfo: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#include <wrappers.h>

int	wrapper_exit_code = 255;

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
  char		runfile[(checkParams(argc,argv),strlen(argv[1])) + sizeof(DEFAULT_PKGSTATEREVDIR "/99999")];
  char		dstfile[PATH_MAX];
  int		fd;
  char		buf[sizeof(int)*3+2];
  xid_t		ctx;
  ssize_t	len;
  ssize_t	len1 = strlen(argv[1]);

  strcpy(runfile,      argv[1]);
  strcpy(runfile+len1, "/run");

  ctx=Evc_get_task_xid(0);

  if (ctx==0) {
    WRITE_MSG(2, "save_ctxinfo: Can not operate in context 0\n");
    return 255;
  }

  if (reinterpret_cast(unsigned int)(ctx)>99999) {
    WRITE_MSG(2, "save_ctxinfo: unexpected context\n");
    return 255;
  }

  len          = EreadlinkD(runfile, dstfile, sizeof(dstfile)-1);
  dstfile[len] = '\0';
  len          = utilvserver_fmt_uint(buf, ctx);

  fd = EopenD(dstfile, O_EXCL|O_CREAT|O_WRONLY, 0644);
  if (write(fd, buf,     len) !=len  ||
      write(fd, "\n",    1)   !=1) {
    perror("save_ctxinfo: write()");
    return -1;
  }
  Eclose(fd);

  strcpy(runfile, DEFAULT_PKGSTATEREVDIR);
  strncat(runfile, buf, len);
  unlink(runfile);
  EsymlinkD(argv[1], runfile);

  Eexecv(argv[2], argv+2);
}
