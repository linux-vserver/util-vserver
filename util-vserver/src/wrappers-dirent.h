// $Id$    --*- c -*--

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


#ifndef H_UTIL_VSERVER_SRC_WRAPPERS_DIRENT_H
#define H_UTIL_VSERVER_SRC_WRAPPERS_DIRENT_H

#include "wrappers.h"
#include <dirent.h>

#define WRAPPER_DECL	UNUSED ALWAYSINLINE

inline static WRAPPER_DECL DIR *
Eopendir(const char *name)
{
  DIR *		res = opendir(name);

  FatalErrnoError(res==0, "opendir()");
  return res;
}

inline static WRAPPER_DECL struct dirent *
Ereaddir(DIR *dir)
{
  struct dirent	*res;

  errno = 0;
  res   = readdir(dir);
  
  FatalErrnoError(res==0 && errno!=0, "readdir()");
  return res;
}

#ifndef __dietlibc__
inline static WRAPPER_DECL void
Ereaddir_r(DIR *dir, struct dirent *entry, struct dirent **result)
{
  errno = 0;
  FatalErrnoError(readdir_r(dir, entry, result)==0 && errno!=0, "readdir_r()");
}
#endif

inline static WRAPPER_DECL void
Eclosedir(DIR *dir)
{
  FatalErrnoError(closedir(dir)==-1, "closedir()");
}

#undef WRAPPER_DECL

#endif	//  H_UTIL_VSERVER_SRC_WRAPPERS_DIRENT_H
