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


#ifndef H_UTIL_VSERVER_COMPAT_H
#define H_UTIL_VSERVER_COMPAT_H

#if defined(__dietlibc__) && !defined(ENSC_DIETLIBC_C99) && defined(__STRICT_ANSI__) && defined(__STDC_VERSION__)
#  include <sys/cdefs.h>
#  undef inline

#  undef  __STRICT_ANSI__
#  include <stdint.h>
#  define __STRICT_ANSI__
#endif

#if defined(__GNUC__)
#  define UNUSED                __attribute__((__unused__))
#  define NORETURN              __attribute__((__noreturn__))
#  if __GNUC__>3 || (__GNUC__==3 && __GNUC__MINOR>=3)
#    define NONNULL(ARGS)	__attribute__((__nonnull__ ARGS))
#    define ALWAYSINLINE        __attribute__((__always_inline__))
#  else
#    define NONNULL(ARGS)
#    define ALWAYSINLINE
#  endif
#else
#  define NONNULL(ARGS)
#  define UNUSED
#  define NORETURN
#  define ALWAYSINLINE
#endif

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__<199901L)
#  define restrict
#endif

#if !defined(HAVE_DECL_MS_MOVE) || !(HAVE_DECL_MS_MOVE)
  // from <linux/fs.h>
#  define MS_MOVE		8192
#endif

#ifndef HAVE_XID_T
#include <stdint.h>
typedef uint32_t		xid_t;
#endif

#if defined(__dietlibc__)
  #define TEMP_FAILURE_RETRY(expression)				\
  (__extension__							\
   ({ long int __result;						\
   do __result = (long int) (expression);				\
   while (__result == -1L && errno == EINTR);				\
  __result; }))
#endif

#define FMT_PREFIX		utilvserver_fmt_

#endif	//  H_UTIL_VSERVER_COMPAT_H
