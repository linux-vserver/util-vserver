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

inline static WRAPPER_DECL void *
Emalloc(size_t size)
{
  register void               *res = malloc(size);
  FatalErrnoError(res==0 && size!=0, "malloc()");
  return res;
}

/*@unused@*/
inline static WRAPPER_DECL /*@null@*//*@only@*/ void *
Erealloc(/*@only@*//*@out@*//*@null@*/ void *ptr,
         size_t new_size)
    /*@ensures maxSet(result) == new_size@*/
    /*@modifies *ptr@*/
{
  register void         *res = realloc(ptr, new_size);
  FatalErrnoError(res==0 && new_size!=0, "realloc()");

  return res;
}
