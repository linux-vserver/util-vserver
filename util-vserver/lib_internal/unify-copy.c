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
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define ENSC_WRAPPERS_IO	1
#include <wrappers.h>

static inline bool
verifySource(int fd, struct stat const *exp_stat)
{
  struct stat		st;
  
  return (fstat(fd, &st)!=-1 &&
	  st.st_dev==exp_stat->st_dev &&
	  st.st_ino==exp_stat->st_ino);
}

static inline bool
copyLnk(char const *src, char const *dst)
{
  ssize_t	len = 1024;
  for (;;) {
    char	buf[len];
    ssize_t	l;
    l = readlink(src, buf, len-1);
    if (l==-1) return false;
    if (l>=len-1) {
      len *= 2;
      continue;
    }
    buf[l] = '\0';

    return (symlink(buf, dst)!=-1);
  }
}

static inline bool
copyReg(char const *src, struct stat const *src_stat,
	char const *dst)
{
  int		in_fd  = open(src, O_RDONLY|O_NOCTTY|O_NONBLOCK|O_NOFOLLOW|O_LARGEFILE);
  int		out_fd = in_fd==-1 ? -1 : open(dst, O_RDWR|O_CREAT|O_EXCL, 0200);
  bool		res    = false;
  
  if (in_fd==-1 || out_fd==-1 ||
      !verifySource(in_fd, src_stat)) goto err;

  for (;;) {
    char	buf[2048];
    ssize_t	l = read(in_fd, buf, sizeof buf);
    if (l==-1) goto err;
    if (l==0)  break;
    if (!WwriteAll(out_fd, buf, l, 0)) goto err;
  }

  res = true;
  
  err:
  if (out_fd!=-1 && close(out_fd)==-1) res=false;
  if (in_fd!=-1  && close(in_fd)==-1)  res=false;
  return res;
}

static inline bool
copyNode(char const UNUSED *src, struct stat const *src_stat,
	 char const *dst)
{
  return mknod(dst, src_stat->st_mode & (S_IFMT|S_IWUSR),
	       src_stat->st_rdev)!=-1;
}

static inline bool
copyDir(char const UNUSED *src, struct stat const UNUSED *src_stat,
	char const *dst)
{
  return mkdir(dst, 0700)!=-1;
}

static inline bool
setModes(char const *path, struct stat const *st)
{
  return (lchown(path, st->st_uid, st->st_gid)!=-1 &&
	  (S_ISLNK(st->st_mode) || chmod(path, st->st_mode)!=-1));
}


bool
Unify_copy(char const *src, struct stat const *src_stat,
	   char const *dst)
{
  // skip sockets
  // TODO: message
  if (S_ISSOCK(src_stat->st_mode))
    return true;
  
  return
    (((S_ISLNK (src_stat->st_mode) && copyLnk (src, dst)) ||
      (S_ISREG (src_stat->st_mode) && copyReg (src, src_stat, dst)) ||
      (S_ISDIR (src_stat->st_mode) && copyDir (src, src_stat, dst)) ||
      ((S_ISBLK (src_stat->st_mode) ||
	S_ISCHR (src_stat->st_mode) || 
	S_ISFIFO(src_stat->st_mode)) && copyNode(src, src_stat, dst))
      ) &&
     setModes(dst, src_stat) &&
     Unify_setTime(dst, src_stat));
}
