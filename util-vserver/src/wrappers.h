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


#ifndef H_UTIL_VSERVER_SRC_WRAPPERS_H
#define H_UTIL_VSERVER_SRC_WRAPPERS_H

#include "compat.h"
#include "compat-pivot_root.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mount.h>

#define WRAPPER_DECL	UNUSED ALWAYSINLINE

static UNUSED void 
FatalErrnoError(bool condition, char const msg[]) /*@*/
{
  if (!condition)       return;
  perror(msg);

  extern int	wrapper_exit_code;
  exit(wrapper_exit_code);
}

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

inline static WRAPPER_DECL int
Eopen(char const *fname, int flags, mode_t mode)
{
  int	res = open(fname, flags, mode);
  FatalErrnoError(res==-1, "open()");

  return res;
}

inline static WRAPPER_DECL void
Eumount2(char const *path, int flag)
{
  FatalErrnoError(umount2(path,flag)==-1, "umount2()");
}

inline static WRAPPER_DECL void
Emount(const char *source, const char *target,
       const char *filesystemtype, unsigned long mountflags,
       const void *data)
{
  FatalErrnoError(mount(source, target, filesystemtype,
			mountflags, data)==-1, "mount()");
}

inline static WRAPPER_DECL void
Emkdir(const char *pathname, mode_t mode)
{
  FatalErrnoError(mkdir(pathname,mode)==-1, "mkdir()");
}

inline static WRAPPER_DECL void
Epivot_root(const char *new_root, const char *put_old)
{
  FatalErrnoError(pivot_root(new_root, put_old)==-1, "pivot_root()");
}

inline static WRAPPER_DECL pid_t
Eclone(int (*fn)(void *), void *child_stack, int flags, void *arg)
{
  pid_t		res;
#ifndef __dietlibc__
  res = clone(fn, child_stack, flags, arg);
#else
  res = clone((void*(*)(void*))(fn), child_stack, flags, arg);
#endif
  FatalErrnoError(res==-1, "clone()");
  return res;
}


inline static WRAPPER_DECL pid_t
Ewait4(pid_t pid, int *status, int options,
       struct rusage *rusage)
{
  pid_t		res;
  res = wait4(pid, status, options, rusage);
  FatalErrnoError(res==-1, "wait4()");
  return res;
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

#undef WRAPPER_DECL

#endif	//  H_UTIL_VSERVER_SRC_WRAPPERS_H
