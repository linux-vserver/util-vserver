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

#include <stdbool.h>

inline static WRAPPER_DECL bool
WsendAll(int fd, void const *ptr_v, size_t len, int *err)
{
  register char const	*ptr = ptr_v;
  if (err) *err = 0;

  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(send(fd, ptr, len, MSG_NOSIGNAL));
    if (res==(size_t)-1) {
      if (err) *err = errno;
      return false;
    }

    if (res==0) return false;

    ptr += res;
    len -= res;
  }
  return true;
}

inline static WRAPPER_DECL void
EsendAll(int fd, void const *ptr_v, size_t len)
{
  register char const	*ptr = ptr_v;

  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(send(fd, ptr, len, MSG_NOSIGNAL));
    FatalErrnoError(res==(size_t)-1, "send()");

    ptr += res;
    len -= res;
  }
}


inline static WRAPPER_DECL bool
WrecvAll(int fd, void *ptr_v, size_t len, int *err)
{
  register char	*ptr = ptr_v;
  if (err) *err = 0;
  
  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(recv(fd, ptr, len, MSG_NOSIGNAL));
    if (res==(size_t)-1) {
      if (err) *err = errno;
      return false;
    }

    if (res==0) return false;

    ptr += res;
    len -= res;
  }
  return true;
}

inline static WRAPPER_DECL bool
ErecvAll(int fd, void *ptr_v, size_t len)
{
  register char	*ptr = ptr_v;
  
  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(recv(fd, ptr, len, MSG_NOSIGNAL));
    FatalErrnoError(res==(size_t)-1, "recv()");

    if (res==0) return false;

    ptr += res;
    len -= res;
  }

  return true;
}
