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


  // secure-mount <chroot-base> <src> <dst>
  //
  // mounts <src> into the chroot <chroot-base> at destination <dst> in a way
  // that prevents symlink attacks

  // algorithm:
  //   enter chroot
  //     mount tmpfs-directory
  //   leave chroot
  //   mount --bind <src> tmpfs
  //   enter chroot
  //     mount --move tmpfs <dst>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define VERSION_COPYRIGHT_YEAR		"2003"
#define VERSION_COPYRIGHT_AUTHOR	"Enrico Scholz"
#include "util.h"

#include <unistd.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define MNTPOINT	"/etc"

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " <chroot-base> <src> <dst>\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "secure-mount 0.23.5\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main(int argc, char *argv[])
{
  int	fd = open("/", O_RDONLY);
    // HACK: sanity checks should be done before declaration, but this does
    // not work with C89 compilers
  char	path[strlen(argc==1 ? "" : argv[1]) + sizeof(MNTPOINT) + 3];

  if (argc>=2) {
    if (!strcmp(argv[1], "--help"))    showHelp(1, argv[0], 0);
    if (!strcmp(argv[1], "--version")) showVersion();
  }
  if (argc!=4) {
    WRITE_MSG(2, "Bad parameter count; use --help for further information.\n");
    exit(255);
  }

  if (chroot(argv[1]) == -1 ||
      chdir("/")      == -1) {
    perror("chroot()/chdir()");
    exit(1);
  }
  
  if (mount("none", MNTPOINT, "tmpfs", 0, "size=4k")==-1) {
    perror("mount()");
    exit(1);
  }

  if (mkdir(MNTPOINT "/x",0600) == -1) {
    perror("mkdir()/fchdir()");
    goto err1;
  }
  
  if (fchdir(fd)      == -1 ||
      chroot(".")     == -1) {
    perror("chroot()");
    goto err2;
  }

  strcpy(path, argv[1]);
  strcat(path, MNTPOINT "/x");
  
  if (mount(argv[2], path, "", MS_BIND, 0)==-1) {
    perror("mount()");
    goto err3;
  }

  if (chroot(argv[1]) == -1) {
    perror("chroot()");
    goto err4;
  }

  if (chdir("/")      == -1 ||
      mount(MNTPOINT "/x", argv[3], "", MS_MOVE, 0) == -1) {
    perror("chdir()/mount()");
    goto err5;
  }

  if (umount(MNTPOINT) == -1) {
    perror("umount()");
  }

  return 0;

  err5:
  err4:
  if (umount(path)==-1)   perror("umount(path)");
  
  err3:
  if (chdir(argv[1])==-1) {
    perror("chdir()");
    return 1;
  }

  err2:
  err1:
  if (umount("mnt")==-1)  perror("umount(\"mnt\")");

  return 1;
}
