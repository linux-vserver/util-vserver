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

#include <stdint.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <errno.h>

#ifndef __NR_vserver
#  define __NR_vserver	273
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

#ifdef VC_ENABLE_API_V11
#  define CALL_VC_V11(F,...)	CALL_VC_GENERAL(0x00010000, v11, F, __VA_ARGS__)
#else
#  define CALL_VC_V11(F,...)	CALL_VC_NOOP
#endif

#if 1
#  define CTX_KERNEL2USER(X)	(((X)==(uint32_t)(-1)) ? VC_NOCTX   : \
				 ((X)==(uint32_t)(-2)) ? VC_SAMECTX : \
				 (xid_t)(X))

#  define CTX_USER2KERNEL(X)	(((X)==VC_RANDCTX) ? (uint32_t)(-1) : \
				 ((X)==VC_SAMECTX) ? (uint32_t)(-2) : \
				 (uint32_t)(X))
#else
#  define CTX_USER2KERNEL(X)	(X)
#  define CTX_KERNEL2USER(X)	(X)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_SYS_VIRTUAL_CONTEXT
#if defined(__pic__) && defined(__i386)
inline static UNUSED ALWAYSINLINE
int vserver(uint32_t cmd, uint32_t id, void *data)
{
  return syscall(__NR_vserver, cmd, id, data);
}
#else
inline static UNUSED ALWAYSINLINE
_syscall3(int, vserver,
	  uint32_t, cmd, uint32_t, id, void *, data)
#endif
#endif

size_t		utilvserver_uint2str(char *buf, size_t len,
				     unsigned int val, unsigned char base);
int		utilvserver_checkCompatVersion();

#ifdef __cplusplus
}
#endif


#endif	//  H_VSERVER_SYSCALL_INTERNAL_H
