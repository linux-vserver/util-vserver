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

#include "lib/syscall-wrap.h"
#define __NR_sys_clone		__NR_clone

#ifndef CLONE_NEWNS
#  define CLONE_NEWNS 0x00020000
#endif

#ifdef ENSC_SYSCALL_TRADITIONAL
#include <unistd.h>

inline static UNUSED ALWAYSINLINE
int sys_clone(int flags, void *stack)
{
#if defined __dietlibc__
  extern long int syscall (long int __sysno, ...);
#endif
 
  return syscall(__NR_sys_clone, flags, stack);
}
#else
#include <errno.h>

inline static UNUSED ALWAYSINLINE
_syscall2(int, sys_clone, int, flags, void *, child_stack)
#endif

#undef __NR_sys_clone

#define ENSC_HAVE_SYSCLONE		1
  
#endif	//  H_UTIL_VSERVER_SRC_SYS_CLONE_H
