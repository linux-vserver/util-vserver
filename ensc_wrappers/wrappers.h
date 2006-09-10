// $Id$    --*- c++ -*--

// Copyright (C) 2003,2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#define WRAPPER_DECL		UNUSED ALWAYSINLINE
#define H_ENSC_IN_WRAPPERS_H	1

#include "wrappers_handler.hc"

#define ENSC_DOQUOTE_COND(PTR, VAL, LEN, DO_QUOTE) \
  if (DO_QUOTE) *PTR++ = '"';					\
  memcpy(PTR, VAL, LEN); PTR += LEN;				\
  if (DO_QUOTE) *PTR++ = '"'					\
  
#define ENSC_DETAIL1(RES,FUNC,VAL,DO_QUOTE)			\
  size_t	l_ = strlen(VAL);				\
  char		RES[l_ + sizeof(FUNC "(\"\")")];		\
  char *	ptr_ = RES;					\
  memcpy(ptr_, FUNC "(", sizeof(FUNC)); ptr_ += sizeof(FUNC);	\
  ENSC_DOQUOTE_COND(ptr_, VAL, l_, DO_QUOTE);			\
  *ptr_++ = ')';						\
  *ptr_   = '\0';

#define ENSC_DETAIL2(RES,FUNC, VAL0,VAL1, DO_QUOTE0,DO_QUOTE1)	\
  size_t	l0_ = strlen(VAL0);				\
  size_t	l1_ = strlen(VAL1);				\
  char		RES[l0_ + l1_ + sizeof(FUNC "('','')")];	\
  char *	ptr_ = RES;					\
  memcpy(ptr_, FUNC "(", sizeof(FUNC)); ptr_ += sizeof(FUNC);	\
  ENSC_DOQUOTE_COND(ptr_, VAL0, l0_, DO_QUOTE0);		\
  *ptr_++ = ',';						\
  ENSC_DOQUOTE_COND(ptr_, VAL1, l1_, DO_QUOTE1);		\
  *ptr_++ = ')';						\
  *ptr_   = '\0';
  

#ifdef ENSC_WRAPPERS_UNISTD
#  include "wrappers-unistd.hc"
#endif

#ifdef ENSC_WRAPPERS_FCNTL
#  include "wrappers-fcntl.hc"
#endif

#ifdef ENSC_WRAPPERS_MOUNT
#  include "wrappers-mount.hc"
#endif

#ifdef ENSC_WRAPPERS_RESOURCE
#  include "wrappers-resource.hc"
#endif

#ifdef ENSC_WRAPPERS_IOCTL
#  include "wrappers-ioctl.hc"
#endif

#ifdef ENSC_WRAPPERS_WAIT
#  include "wrappers-wait.hc"
#endif

#ifdef ENSC_WRAPPERS_VSERVER
#  include "wrappers-vserver.hc"
#endif

#ifdef ENSC_WRAPPERS_IO
#  include "wrappers-io.hc"
#endif

#ifdef ENSC_WRAPPERS_IOSOCK
#  include "wrappers-iosock.hc"
#endif

#ifdef ENSC_WRAPPERS_DIRENT
#  include "wrappers-dirent.hc"
#endif

#ifdef ENSC_WRAPPERS_CLONE
#  include "wrappers-clone.hc"
#endif

#ifdef ENSC_WRAPPERS_STDLIB
#  include "wrappers-stdlib.hc"
#endif

#ifdef ENSC_WRAPPERS_STRING
#  include "wrappers-string.hc"
#endif

#ifdef ENSC_WRAPPERS_SOCKET
#  include "wrappers-socket.hc"
#endif

#ifdef ENSC_WRAPPERS_STAT
#  include "wrappers-stat.hc"
#endif

#undef ENSC_DETAIL2
#undef ENSC_DETAIL1
#undef ENSC_DOQUOTE_COND
#undef H_ENSC_IN_WRAPPERS_H
#undef WRAPPER_DECL

#endif	//  H_UTIL_VSERVER_SRC_WRAPPERS_H
