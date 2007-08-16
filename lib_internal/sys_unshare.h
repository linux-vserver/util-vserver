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

#ifndef __NR_unshare
#  if defined(__alpha__)
#    define __NR_unshare	465
#  elif defined(__arm__)
#    define __NR_unshare	337
#  elif defined(__avr32__)
#    define __NR_unshare	258
#  elif defined(__blackfin__)
#    define __NR_unshare	310
#  elif defined(__frv__)
#    define __NR_unshare	310
#  elif defined(__h8300__)
#    define __NR_unshare	310
#  elif defined(__i386__)
#    define __NR_unshare	310
#  elif defined(__ia64__)
#    define __NR_unshare	1296
#  elif defined(__m68knommu__)
#    define __NR_unshare	303
#  elif defined(__m68k__)
#    define __NR_unshare	303
#  elif defined(__mips__) && (_MIPS_SIM == _MIPS_SIM_ABI32)
#    define __NR_unshare	303
#  elif defined(__mips__) && (_MIPS_SIM == _MIPS_SIM_ABI64)
#    define __NR_unshare	262
#  elif defined(__mips__) && (_MIPS_SIM == _MIPS_SIM_NABI32)
#    define __NR_unshare	266
#  elif defined(__parisc__)
#    define __NR_unshare	288
#  elif defined(__powerpc__)
#    define __NR_unshare	282
#  elif defined(__s390__)
#    define __NR_unshare	303
#  elif defined(__sh64__)
#    define __NR_unshare	338
#  elif defined(__sh__)
#    define __NR_unshare	310
#  elif defined(__sparc64__)
#    define __NR_unshare	299
#  elif defined(__sparc__)
#    define __NR_unshare	299
#  elif defined(__x86_64__)
#    define __NR_unshare	272
#  else
#    error Sorry, don't know unshare's syscall number for this architecture.
#  endif
#endif

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
