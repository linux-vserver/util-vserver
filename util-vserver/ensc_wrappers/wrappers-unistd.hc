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

#ifndef H_ENSC_IN_WRAPPERS_H
#  error wrappers_handler.hc can not be used in this way
#endif

inline static WRAPPER_DECL void
Eclose(int s)
{
  FatalErrnoError(close(s)==-1, "close()");
}

inline static WRAPPER_DECL void
Echdir(char const path[])
{
  FatalErrnoError(chdir(path)==-1, "chdir()");
}

inline static WRAPPER_DECL void
Efchdir(int fd)
{
  FatalErrnoError(fchdir(fd)==-1, "fchdir()");
}

inline static WRAPPER_DECL void
Echroot(char const path[])
{
  FatalErrnoError(chroot(path)==-1, "chroot()");
}

inline static WRAPPER_DECL void
Eexecv(char const *path, char *argv[])
{
  FatalErrnoError(execv(path,argv)==-1, "execv()");
}

inline static WRAPPER_DECL void
Epipe(int filedes[2])
{
  FatalErrnoError(pipe(filedes)==-1, "pipe()");
}

inline static WRAPPER_DECL pid_t
Efork()
{
  pid_t		res;
  res = fork();
  FatalErrnoError(res==-1, "fork()");
  return res;
}

inline static WRAPPER_DECL size_t
Eread(int fd, void *ptr, size_t len)
{
  size_t	res = read(fd, ptr, len);
  FatalErrnoError((ssize_t)(res)==-1, "read()");

  return res;
}

inline static WRAPPER_DECL size_t
Ewrite(int fd, void const *ptr, size_t len)
{
  size_t	res = write(fd, ptr, len);
  FatalErrnoError((ssize_t)(res)==-1, "write()");

  return res;
}

inline static WRAPPER_DECL void
Ereadlink(const char *path, char *buf, size_t bufsiz)
{
  FatalErrnoError(readlink(path, buf, bufsiz)==-1, "readlink()");
}

inline static WRAPPER_DECL void
Esymlink(const char *oldpath, const char *newpath)
{
  FatalErrnoError(symlink(oldpath, newpath)==-1, "symlink()");
}

inline static WRAPPER_DECL void
Eunlink(char const *pathname)
{
  FatalErrnoError(unlink(pathname)==-1, "unlink()");
}

inline static void
Esetuid(uid_t uid)
{
  FatalErrnoError(setuid(uid)==-1, "setuid()");
}

inline static void
Esetgid(gid_t gid)
{
  FatalErrnoError(setgid(gid)==-1, "setgid()");
}

#if defined(_GRP_H) && (defined(__USE_BSD) || defined(__dietlibc__))
inline static void
Esetgroups(size_t size, const gid_t *list)
{
  FatalErrnoError(setgroups(size, list)==-1, "setgroups()");
}
#endif

inline static WRAPPER_DECL int
Edup2(int oldfd, int newfd)
{
  register int          res = dup2(oldfd, newfd);
  FatalErrnoError(res==-1, "dup2()");

  return res;
}

inline static WRAPPER_DECL pid_t
Esetsid()
{
  register pid_t const  res = setsid();
  FatalErrnoError(res==-1, "setsid()");

  return res;
}

inline static WRAPPER_DECL int
Emkstemp(char *template)
{
  int		res = mkstemp(template);
  FatalErrnoError(res==-1, "mkstemp()");
  return res;
}

inline static WRAPPER_DECL off_t
Elseek(int fildes, off_t offset, int whence)
{
  off_t         res = lseek(fildes, offset, whence);
  FatalErrnoError(res==(off_t)-1, "lseek()");
  return res;
}
