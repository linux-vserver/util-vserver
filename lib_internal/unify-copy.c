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
#include "util.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define ENSC_WRAPPERS_IO	1
#include <wrappers.h>

#define MMAP_BLOCKSIZE		(16 * 1024*1024)

#ifndef   TESTSUITE_COPY_CODE
#  define TESTSUITE_COPY_CODE	do { } while (false)
#endif

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

static sigjmp_buf		bus_error_restore;
static volatile sig_atomic_t 	bus_error;

static void
handlerSIGBUS(int UNUSED num)
{
  bus_error = 1;
  siglongjmp(bus_error_restore, 1);
}

static void
copyMem(void *dst_v, void const *src_v, size_t len_v)
{
#if 1
    // Do not use memcpy because this would dirty pages consisting only of
    // '\0'
  int		*dst = dst_v;
  int const	*src = src_v;
  size_t	len  = len_v / sizeof(int);
  size_t	rest = len_v - sizeof(int)*len;
  size_t	i=0;

  for (; i<len; ++i) {
    if (*src != 0) *dst = *src;
    ++dst;
    ++src;
  }

  char		*dst_c = (void *)(dst);
  char const	*src_c = (void const *)(src);

  for (i=0; i<rest; ++i) {
    if (*src_c != 0) *dst_c = *src_c;
    ++dst_c;
    ++src_c;
  }
#else
  memcpy(dst_v, src_v, len_v);
#endif  
}

static UNUSED bool
copyMMap(int in_fd, int out_fd)
{
  off_t			in_len   = lseek(in_fd, 0, SEEK_END);
  void const * volatile in_buf   = 0;
  void       * volatile	out_buf  = 0;
  
  loff_t volatile	buf_size = 0;
  bool   volatile 	res      = false;

  if (in_len==-1) return false;
  if (in_len>0 && ftruncate(out_fd, in_len)==-1)	// create sparse file
    return false;
  
  bus_error = 0;
  if (sigsetjmp(bus_error_restore, 1)==0) {
    off_t		offset   = 0;

    while (offset < in_len) {
      buf_size = in_len - offset;
      if (buf_size > MMAP_BLOCKSIZE) buf_size = MMAP_BLOCKSIZE;
      
      if ((in_buf  = mmap(0, buf_size, PROT_READ,  MAP_SHARED,  in_fd, offset))==MAP_FAILED ||
	  (out_buf = mmap(0, buf_size, PROT_WRITE, MAP_SHARED, out_fd, offset))==MAP_FAILED) {
	perror("mmap()");
	goto out;
      }

      offset  += buf_size;
      madvise(const_cast(void *)(in_buf),  buf_size, MADV_SEQUENTIAL);
      madvise(out_buf,                     buf_size, MADV_SEQUENTIAL);

      TESTSUITE_COPY_CODE;
      copyMem(out_buf, in_buf, buf_size);

      munmap(out_buf,                     buf_size); out_buf = 0;
      munmap(const_cast(void *)(in_buf),  buf_size);  in_buf = 0;
    }

    res = true;
  }

  out:
  if (out_buf!=0) munmap(out_buf,                     buf_size);
  if (in_buf !=0) munmap(const_cast(void *)(in_buf),  buf_size);

  return res;
}

static inline bool
copyReg(char const *src, struct stat const *src_stat,
	char const *dst)
{
  int		in_fd  = open(src, O_RDONLY|O_NOCTTY|O_NONBLOCK|O_NOFOLLOW|O_LARGEFILE);
  int		out_fd = in_fd==-1 ? -1 : open(dst, O_RDWR|O_CREAT|O_EXCL|O_NOCTTY, 0200);
  bool		res    = false;
  
  if (in_fd==-1 || out_fd==-1 ||
      !verifySource(in_fd, src_stat)) goto err;

#if 0  
  for (;;) {
    char	buf[2048];
    ssize_t	l = read(in_fd, buf, sizeof buf);
    if (l==-1) goto err;
    if (l==0)  break;
    if (!WwriteAll(out_fd, buf, l, 0)) goto err;
  }

  res = true;
#else
  void		(*old_handler)(int) = signal(SIGBUS, handlerSIGBUS);

  res = copyMMap(in_fd, out_fd);

  signal(SIGBUS, old_handler);
#endif

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
