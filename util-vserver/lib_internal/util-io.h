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


#ifndef H_UTIL_VSERVER_LIB_INTERNAL_UTIL_IO_H
#define H_UTIL_VSERVER_LIB_INTERNAL_UTIL_IO_H

#include <unistd.h>
#include <string.h>

inline static void UNUSED
Vwrite(int fd, char const *buf, size_t len)
{
  if (write(fd,buf,len)==-1) { /**/ }
}

inline static void UNUSED
writeStr(int fd, char const *cmd)
{
  Vwrite(fd, cmd, strlen(cmd));
}

#define WRITE_MSG(FD,X)		Vwrite(FD,X,sizeof(X)-1)
#define WRITE_STR(FD,X)		writeStr(FD,X)


#endif	//  H_UTIL_VSERVER_LIB_INTERNAL_UTIL_IO_H
