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


#ifndef H_UTIL_VSERVER_SRC_COMPAT_PIVOT_ROOT_H
#define H_UTIL_VSERVER_SRC_COMPAT_PIVOT_ROOT_H

#include <sys/syscall.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <errno.h>

inline static _syscall2(int,pivot_root,const char *,new_root,const char *,put_old)

#endif	//  H_UTIL_VSERVER_SRC_COMPAT_PIVOT_ROOT_H
