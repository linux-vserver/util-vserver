// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#ifndef H_ENSC_IN_WRAPPERS_H
#  error wrappers-stat.hc can not be used in this way
#endif

#define ENSC_STAT_DECL(FUNC)					\
  inline static WRAPPER_DECL void				\
  E##FUNC(char const *filename, struct stat *buf)		\
  {								\
    FatalErrnoError(FUNC(filename, buf)==-1, #FUNC "()");	\
  }								\
								\
  inline static WRAPPER_DECL void				\
  E##FUNC##D(char const *filename, struct stat *buf)		\
  {								\
    ENSC_DETAIL1(msg, #FUNC, filename, 1);			\
    FatalErrnoError(FUNC(filename, buf)==-1, msg);		\
  }


ENSC_STAT_DECL(stat)
ENSC_STAT_DECL(lstat)

#undef ENSC_STAT_DECL

  inline static WRAPPER_DECL void				
Efstat(int fd, struct stat *buf)		
{								
  FatalErrnoError(fstat(fd, buf)==-1, "fstat()");	
}
