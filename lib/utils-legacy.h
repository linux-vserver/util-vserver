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


#ifndef H_UTIL_VSERVER_LIB_UTILS_COMPAT_H
#define H_UTIL_VSERVER_LIB_UTILS_COMPAT_H

#include <stdlib.h>
#include <sys/types.h>

  // read /proc/<pid>/status into 'buf' which has the size bufsize. When 'str'
  // is non-null, search this string and return a pointer *after* it. When no
  // such string could be found, return 0.  When 'str' is null return 'buf'.
  //
  // When this function fails (result==0) and errno is EAGAIN, the buffersize
  // was too small and this function should be called again with a larger
  // buffer.
char *		utilvserver_getProcEntry(pid_t pid, char *str,
					 char *buf, size_t bufsize);

  // Returns the suggested buffersize for reading a /proc/.../status
  // file. Return-value can change when utilvserver_getProcEntry() was called.
size_t		utilvserver_getProcEntryBufsize();

#endif	//  H_UTIL_VSERVER_LIB_UTILS_COMPAT_H
