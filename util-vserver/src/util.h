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


#ifndef H_UTIL_VSERVER_SRC_UTIL_H
#define H_UTIL_VSERVER_SRC_UTIL_H

#include "compat.h"
#include "lib_internal/util-io.h"
#include "lib_internal/util-mem.h"
#include "lib_internal/util-safechdir.h"
#include "lib_internal/util-dotfile.h"

#include <unistd.h>
#include <string.h>

#ifndef PACKAGE_BUGREPORT
#  define PACKAGE_BUGREPORT	"???"
#endif

#ifndef VERSION_COPYRIGHT_DISCLAIMER
#  define VERSION_COPYRIGHT_DISCLAIMER	\
  "This program is free software; you may redistribute it under the terms of\n" \
  "the GNU General Public License.  This program has absolutely no warranty.\n"
#endif

#ifndef __cplusplus
#  define cAsT_(X)              (X))
#  define reinterpret_cast(X)   ((X) cAsT_
#  define static_cast(X)        ((X) cAsT_
#  define const_cast(X)         ((X) cAsT_
#else   /* __cplusplus */
#  define reinterpret_cast(X)   reinterpret_cast<X>
#  define static_cast(X)        static_cast<X>
#  define const_cast(X)         const_cast<X>
#endif

#define DIM_OF(X)		(sizeof(X)/sizeof((X)[0]))

void	exitLikeProcess(int pid) NORETURN;


#define VSERVER_DECLARE_CMD(CMD)     \
  char		buf[strlen(CMD)+1];  \
  memcpy(buf, (CMD), strlen(CMD)+1); \
  CMD = basename(buf);

#endif	//  H_UTIL_VSERVER_SRC_UTIL_H
