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


#ifndef H_VSERVER_DJINNI_SRC_WRAPPERS_IO_H
#define H_VSERVER_DJINNI_SRC_WRAPPERS_IO_H

#include "wrappers.h"

#ifdef UTILVSERVER_ENABLE_SOCKET_WRAPPERS
inline static UNUSED bool
WsendAll(int fd, void const *ptr_v, size_t len)
{
  register char const	*ptr = ptr_v;

  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(send(fd, ptr, len, MSG_NOSIGNAL));
    if (res==(size_t)-1) {
      perror("send()");
      return false;
    }

    if (res==0) return false;

    ptr += res;
    len -= res;
  }
  return true;
}

inline static UNUSED void
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


inline static UNUSED bool
WrecvAll(int fd, void *ptr_v, size_t len)
{
  register char	*ptr = ptr_v;
  
  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(recv(fd, ptr, len, MSG_NOSIGNAL));
    if (res==(size_t)-1) {
      perror("recv()");
      return false;
    }

    if (res==0) return false;

    ptr += res;
    len -= res;
  }
  return true;
}

inline static UNUSED bool
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
#endif

inline static UNUSED bool
WwriteAll(int fd, void const *ptr_v, size_t len)
{
  register char const	*ptr = ptr_v;

  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(write(fd, ptr, len));
    if (res==(size_t)-1) {
      perror("write()");
      return false;
    }

    if (res==0) return false;

    ptr += res;
    len -= res;
  }
  return true;
}

inline static UNUSED void
EwriteAll(int fd, void const *ptr_v, size_t len)
{
  register char const	*ptr = ptr_v;

  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(write(fd, ptr, len));
    FatalErrnoError(res==(size_t)-1, "write()");

    ptr += res;
    len -= res;
  }
}


inline static UNUSED bool
WreadAll(int fd, void *ptr_v, size_t len)
{
  register char	*ptr = ptr_v;
  
  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(read(fd, ptr, len));
    if (res==(size_t)-1) {
      perror("read()");
      return false;
    }

    if (res==0) return false;

    ptr += res;
    len -= res;
  }
  return true;
}

inline static UNUSED bool
EreadAll(int fd, void *ptr_v, size_t len)
{
  register char	*ptr = ptr_v;
  
  while (len>0) {
    size_t	res = TEMP_FAILURE_RETRY(read(fd, ptr, len));
    FatalErrnoError(res==(size_t)-1, "read()");

    if (res==0) return false;

    ptr += res;
    len -= res;
  }

  return true;
}

#endif	//  H_VSERVER_DJINNI_SRC_WRAPPERS_IO_H
