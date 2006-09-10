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
#  error wrappers-socket.hc can not be used in this way
#endif

inline static WRAPPER_DECL int
Esocket(int domain, int type, int protocol)
{
  register int		res = socket(domain, type, protocol);
  FatalErrnoError(res==-1, "socket()");
  return res;
}

inline static WRAPPER_DECL void
Econnect(int sockfd, void const *serv_addr, socklen_t addrlen)
{
  FatalErrnoError(connect(sockfd, serv_addr, addrlen)==-1, "connect()");
}

inline static WRAPPER_DECL void
Ebind(int sockfd, void *my_addr, socklen_t addrlen)
{
  FatalErrnoError(bind(sockfd, my_addr, addrlen)==-1, "bind()");
}

inline static WRAPPER_DECL int
Eaccept(int s, void *addr, socklen_t *addrlen)
{
  register int		res = accept(s,addr,addrlen);
  FatalErrnoError(res==-1, "accept()");
  return res;
}

inline static WRAPPER_DECL void
Elisten(int sock, int backlog)
{
  FatalErrnoError(listen(sock, backlog)==-1, "bind()");
}

inline static WRAPPER_DECL void
Eshutdown(int s, int how)
{
  FatalErrnoError(shutdown(s,how)==-1, "shutdown()");
}

inline static WRAPPER_DECL ssize_t
Erecv(int s, void *buf, size_t len, int flags)
{
  register ssize_t	res = recv(s,buf,len,flags);
  FatalErrnoError(res==-1, "recv()");
  return res;
}

inline static WRAPPER_DECL ssize_t
Esend(int s, void const *buf, size_t len, int flags)
{
  register ssize_t	res = send(s,buf,len,flags);
  FatalErrnoError(res==-1, "send()");
  return res;
}

inline static WRAPPER_DECL int
Eselect(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	struct timeval *timeout)
{
  register int		res = select(n, readfds,writefds,exceptfds, timeout);
  FatalErrnoError(res==-1, "select()");
  return res;
}

inline static WRAPPER_DECL void
Esocketpair(int d, int type, int protocol, int sv[2])
{
  FatalErrnoError(socketpair(d,type,protocol,sv)==-1, "socketpair()");
}
