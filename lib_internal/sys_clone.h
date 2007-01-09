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


#ifndef H_UTIL_VSERVER_SRC_SYS_CLONE_H
#define H_UTIL_VSERVER_SRC_SYS_CLONE_H

#include <unistd.h>
#include "lib/syscall-wrap.h"
#define __NR__sys_clone		__NR_clone

#ifndef ENSC_SYSCALL_TRADITIONAL
#  include <errno.h>

#  if defined(__s390__)
inline static UNUSED ALWAYSINLINE
_syscall2(int, _sys_clone, void *, child_stack, int, flags)
#  else
inline static UNUSED ALWAYSINLINE
_syscall2(int, _sys_clone, int, flags, void *, child_stack)
#  endif
#endif

inline static UNUSED ALWAYSINLINE
int sys_clone(int flags, void *child_stack)
{
  int ret;
#ifdef __sparc__
  int parent = getpid();
#endif

#if   defined(__s390__) && defined(ENSC_SYSCALL_TRADITIONAL)
  ret = syscall(__NR__sys_clone, child_stack, flags);
#elif defined(__s390__)
  ret = _sys_clone(child_stack, flags);
#elif defined(ENSC_SYSCALL_TRADITIONAL)
  ret = syscall(__NR__sys_clone, flags, child_stack);
#else
  ret = _sys_clone(flags, child_stack);
#endif
#ifdef __sparc__
  if (ret == parent)
    ret = 0;
#endif
  return ret;
}

#undef __NR__sys_clone

#define ENSC_HAVE_SYSCLONE		1
  
#endif	//  H_UTIL_VSERVER_SRC_SYS_CLONE_H
