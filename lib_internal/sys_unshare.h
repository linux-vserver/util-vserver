// $Id$    --*- c -*--

// Copyright (C) 2007 Daniel Hokka Zakrisson
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


#ifndef H_UTIL_VSERVER_SRC_SYS_UNSHARE_H
#define H_UTIL_VSERVER_SRC_SYS_UNSHARE_H

#include <unistd.h>
#include "lib/syscall-wrap.h"
#define __NR_sys_unshare	__NR_unshare

#ifndef ENSC_SYSCALL_TRADITIONAL
#  include <errno.h>

inline static UNUSED ALWAYSINLINE
_syscall1(int, sys_unshare, int, flags)
#else
inline static UNUSED ALWAYSINLINE
int sys_unshare(int flags)
{
  return syscall(__NR_sys_clone, flags);
}
#endif

#undef __NR_sys_unshare

#define ENSC_HAVE_SYSUNSHARE		1
  
#endif	//  H_UTIL_VSERVER_SRC_SYS_UNSHARE_H
