// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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


#ifndef H_UTIL_VSERVER_LIB_SYSCALL_WRAP_H
#define H_UTIL_VSERVER_LIB_SYSCALL_WRAP_H

#include <sys/syscall.h>
#include <syscall.h>
#include <unistd.h>

#ifdef ENSC_USE_ALTERNATIVE_SYSCALL_MACROS
#  undef _syscall0
#  undef _syscall1
#  undef _syscall2
#  undef _syscall3
#  undef _syscall4
#  undef _syscall5
#  undef _syscall6
#  undef _syscall7

#  include "syscall-alternative.h"
#endif

#if defined(ENSC_SYSCALL_TRADITIONAL) && defined(__dietlibc__) && !defined(ENSC_DIETLIBC_HAS_SYSCALL)
extern long int syscall(long int __sysno, ...);
#endif

#endif	//  H_UTIL_VSERVER_LIB_SYSCALL_WRAP_H
