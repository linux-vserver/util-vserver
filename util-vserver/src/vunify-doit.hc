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

#include <sys/types.h>
#include <utime.h>
#include <fcntl.h>

static bool
doitUnify(char const *src, struct stat const *src_stat,
	  char const *dst, struct stat const UNUSED *dst_stat)
{
  size_t	l = strlen(dst);
  char		tmpfile[l + sizeof(";XXXXXX")];
  int		fd;
  bool		res = false;

  // at first, set the ILI flags on 'src'
  if (vc_set_iattr_compat(src, src_stat->st_dev, src_stat->st_ino,
			  0, VC_IATTR_IUNLINK, VC_IATTR_IUNLINK,
			  &src_stat->st_mode)==-1)
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

static bool
doitDeUnify(char const UNUSED *src, struct stat const UNUSED *src_stat,
	    char const *dst,        struct stat const UNUSED *dst_stat)
{
  size_t		l = strlen(dst);
  char			tmpfile[l + sizeof(";XXXXXX")];
  int			fd_src, fd_tmp;
  struct stat		st;
  struct utimbuf	utm;

  fd_src = open(dst, O_RDONLY);
  if (fd_src==-1) {
    perror("open()");
    return false;
  }

  if (fstat(fd_src, &st)==-1) {
    perror("fstat()");
    close(fd_src);
    return false;
  }
  
  memcpy(tmpfile,   dst, l);
  memcpy(tmpfile+l, ";XXXXXX", 8);
  fd_tmp = mkstemp(tmpfile);

  if (fd_tmp==-1) {
    perror("mkstemp()");
    tmpfile[0] = '\0';
    goto err;
  }

  if (fchown(fd_tmp, st.st_uid, st.st_gid)==-1 ||
      fchmod(fd_tmp, st.st_mode)==-1) {
    perror("fchown()/fchmod()");
    goto err;
  }

  // todo: acl?

  for (;;) {
    char	buf[0x4000];
    ssize_t	len = read(fd_src, buf, sizeof buf);
    if (len==-1) {
      perror("read()");
      goto err;
    }
    if (len==0) break;

    if (!WwriteAll(fd_tmp, buf, len)) goto err;
  }

  if (close(fd_src)==-1) {
    perror("close()");
    goto err;
  }
  if (close(fd_tmp)==-1) {
    perror("close()");
    goto err;
  }
  
  utm.actime  = st.st_atime;
  utm.modtime = st.st_mtime;

  // ALERT: race !!!
  if (utime(tmpfile, &utm)==-1) {
    perror("utime()");
    goto err1;
  }

  if (unlink(dst)==-1) {
    perror("unlink()");
    goto err1;
  }
  
  // ALERT: race !!!
  if (rename(tmpfile, dst)==-1) {
    perror("FATAL error in rename()");
    _exit(1);
  }

  return true;
  
  err:
  close(fd_src);
  close(fd_tmp);
  err1:
  if (tmpfile[0]) unlink(tmpfile);

  return false;
}
