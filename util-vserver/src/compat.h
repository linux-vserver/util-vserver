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


#ifndef H_UTIL_VSERVER_SRC_COMPAT_H
#define H_UTIL_VSERVER_SRC_COMPAT_H

#if defined(__GNUC__)
#  define UNUSED                __attribute__((__unused__))
#  define NORETURN              __attribute__((__noreturn__))
#  if __GNUC__ >= 3
#    define ALWAYSINLINE        __attribute__((__always_inline__))
#  else
#    define ALWAYSINLINE
#  endif
#else
#  define UNUSED
#  define NORETURN
#  define ALWAYSINLINE
#endif

#endif	//  H_UTIL_VSERVER_SRC_COMPAT_H
