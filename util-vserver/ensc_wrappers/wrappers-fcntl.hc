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
#  error wrappers_handler.hc can not be used in this way
#endif

inline static WRAPPER_DECL int
Eopen(char const *fname, int flags, mode_t mode)
{
  int	res = open(fname, flags, mode);
  FatalErrnoError(res==-1, "open()");

  return res;
}

inline static WRAPPER_DECL int
EopenD(char const *fname, int flags, mode_t mode)
{
  ENSC_DETAIL1(msg, "open", fname, 1);
  
  {
    int		res = open(fname, flags, mode);
    FatalErrnoError(res==-1, msg);
    return res;
  }
}


inline static WRAPPER_DECL void
Emkdir(const char *pathname, mode_t mode)
{
  FatalErrnoError(mkdir(pathname,mode)==-1, "mkdir()");
}

inline static WRAPPER_DECL void
EmkdirD(const char *pathname, mode_t mode)
{
  ENSC_DETAIL1(msg, "mkdir", pathname, 1);
  FatalErrnoError(mkdir(pathname,mode)==-1, msg);
}
