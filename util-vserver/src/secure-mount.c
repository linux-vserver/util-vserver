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


  // secure-mount <secure-base> <src> <dst>
  //
  // mounts <src> into the chroot <secure-base> at destination <dst> in a way
  // that prevents symlink attacks


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define VERSION_COPYRIGHT_YEAR		"2003"
#define VERSION_COPYRIGHT_AUTHOR	"Enrico Scholz"
#include "util.h"

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fs.h>

#define MNTPOINT	"/etc"

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " <secure-base> <src> <dst>\n\n"
	    "Mounts <src> to <secure-base>/<dst> and assumes <secure-base>\n"
	    "as trustworthy but <dst> as hostile. Therefore, <dst> will be\n"
	    "traversed in a secure way and must not contain symlinks.\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "secure-mount " VERSION " -- secure mounting of directories\n"
	    "This program is part of " PACKAGE_STRING "\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

inline static bool
isSameObject(struct stat const *lhs,
	     struct stat const *rhs)
{
  return (lhs->st_dev==rhs->st_dev &&
	  lhs->st_ino==rhs->st_ino);
}

static int
chdirSecure(char const *dir)
{
  char		tmp[strlen(dir)+1], *ptr;
  char const	*cur;

  strcpy(tmp, dir);
  cur = strtok_r(tmp, "/", &ptr);
  while (cur) {
    struct stat		pre_stat, post_stat;

    if (lstat(cur, &pre_stat)==-1) return -1;
    
    if (!S_ISDIR(pre_stat.st_mode)) {
      errno = ENOENT;
      return -1;
    }
    if (S_ISLNK(pre_stat.st_mode)) {
      errno = EINVAL;
      return -1;
    }

    if (chdir(cur)==-1)            return -1;
    if (stat(".", &post_stat)==-1) return -1;

    if (!isSameObject(&pre_stat, &post_stat)) {
      char	dir[PATH_MAX];
      
      WRITE_MSG(2, "Possible symlink race ATTACK at '");
      WRITE_STR(2, getcwd(dir, sizeof(dir)));
      WRITE_MSG(2, "'\n");

      errno = EINVAL;
      return -1;
    }

    cur = strtok_r(0, "/", &ptr);
  }

  return 0;
}

static int
verifyPosition(char const *mntpoint, char const *dir1, char const *dir2)
{
  struct stat		pre_stat, post_stat;

  if (stat(mntpoint, &pre_stat)==-1)       return -1;
  if (chroot(dir1)==-1 || chdir(dir2)==-1) return -1;
  if (stat(".", &post_stat)==-1)           return -1;

  if (!isSameObject(&pre_stat, &post_stat)) {
    char	dir[PATH_MAX];
      
    WRITE_MSG(2, "Possible symlink race ATTACK at '");
    WRITE_STR(2, getcwd(dir, sizeof(dir)));
    WRITE_MSG(2, "' within '");
    WRITE_MSG(2, dir1);
    WRITE_STR(2, "'\n");

    errno = EINVAL;
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  if (argc>=2) {
    if (!strcmp(argv[1], "--help"))    showHelp(1, argv[0], 0);
    if (!strcmp(argv[1], "--version")) showVersion();
  }
  if (argc!=4) {
    WRITE_MSG(2, "Bad parameter count; use --help for further information.\n");
    exit(255);
  }

  if (chdir(argv[1])==-1) {
    perror("chdir()");
    return 1;
  }

  if (chdirSecure(argv[3])==-1) {
    perror("chdirSecure()");
    return 1;
  }

  if (mount(argv[2], ".", "", MS_BIND, 0)==-1) {
    perror("mount()");
    return 1;
  }

    // Check if directories were moved between the chdirSecure() and mount(2)
  if (verifyPosition(argv[2], argv[1], argv[3])==-1) {
      // TODO: what is with unmounting?
    return 1;
  }

  return 0;
}
