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


#ifndef H_VSERVER_SYSCALL_INTERNAL_H
#define H_VSERVER_SYSCALL_INTERNAL_H

#include <asm/unistd.h>
#include <syscall.h>
#include <errno.h>
#include <stdint.h>

#ifndef __NR_sys_virtual_context
#  define __NR_sys_virtual_context	273
#endif

#define VC_PREFIX	0)
#define VC_SUFFIX	else (void)((void)0
#define CALL_VC_NOOP	(void)0
#define CALL_VC_GENERAL(ID, SUFFIX, FUNC, ...)				\
  VC_PREFIX; VC_SELECT(ID) return FUNC ## _ ## SUFFIX(__VA_ARGS__); VC_SUFFIX

#if 1
#  define VC_SELECT(ID)	case ID: if(1)
#  define CALL_VC(...)					\
  switch (utilvserver_checkCompatVersion()&~0xff) {	\
    case -1	:  if (1) break;			\
      VC_SUFFIX, __VA_ARGS__ , VC_PREFIX;		\
    default	:  errno = EINVAL;			\
  }							\
  return -1
#else
#  define VC_SELECT(ID) if (1)
#  define CALL_VC(...)				\
  if (1) {} VC_SUFFIX, __VA_ARGS__, VC_PREFIX;	\
  errno = ENOSYS; return -1
#endif

#ifdef VC_ENABLE_API_COMPAT
#  define CALL_VC_COMPAT(F,...) CALL_VC_GENERAL(0x00010000, compat, F, __VA_ARGS__)
#else
#  define CALL_VC_COMPAT(F,...)	CALL_VC_NOOP
#endif

#ifdef VC_ENABLE_API_LEGACY
#  define CALL_VC_LEGACY(F,...) CALL_VC_GENERAL(0x00000000, legacy, F, __VA_ARGS__)
#else
#  define CALL_VC_LEGACY(F,...) CALL_VC_NOOP
#endif

#ifndef HAVE_SYS_VIRTUAL_CONTEXT
static UNUSED
_syscall3(int, sys_virtual_context,
	  uint32_t, cmd, uint32_t, id, void *, data)
#endif

#endif	//  H_VSERVER_SYSCALL_INTERNAL_H
