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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "interface.h"
#include "pathconfig.h"

#include <lib_internal/command.h>
#include <lib_internal/util.h>
#include <ensc_fmt/fmt.h>

#include <netinet/in.h>
#include <arpa/inet.h>

unsigned int
Iface_getIPv4Prefix(struct Interface const *iface)
{
  uint32_t	mask = iface->addr.ipv4.mask;
  unsigned int	res  = 0;
  while (mask!=0) {
    res += mask & 1;
    mask >>= 1;
  }

  return res;
}

static bool
invokeIpAddr(struct Interface const *iface)
{
  struct Command		cmd;
  unsigned int			prefix = Iface_getIPv4Prefix(iface);
  char				buf[sizeof("255.255.255.255/") + sizeof(unsigned int)*3 + 1];
  char *			tmp = inet_ntoa(*reinterpret_cast(struct in_addr *)(&iface->addr.ipv4.ip));
  size_t			l   = strlen(tmp);
  char *			ptr;

  if (l>=sizeof("255.255.255.255")) {
    abort();
    return false;
  }
  ptr    = Xmemcpy(buf, tmp, l);
  *ptr++ = '/';
  l      = utilvserver_fmt_uint(ptr, prefix);
  ptr[l] = '\0';

  Command_init(&cmd, 5);
  Command_appendParameter(&cmd, "/bin/echo");
  Command_appendParameter(&cmd, PROG_IP);
  Command_appendParameter(&cmd, buf);
  Command_appendParameter(&cmd, "broadcast");
  if (iface->addr.ipv4.bcast!=0)
    Command_appendParameter(&cmd, inet_ntoa(*reinterpret_cast(struct in_addr *)(&iface->addr.ipv4.bcast)));
  else
    Command_appendParameter(&cmd, "+");

  size_t			l1 = strlen(iface->dev);
  size_t			l2 = iface->name ? strlen(iface->name) : 0;
  char				devlabel[l1 + l2 + sizeof(":")];
  
  if (iface->name) {
    ptr    = Xmemcpy(devlabel, iface->dev,  l1);
    *ptr++ = ':';
    ptr    = Xmemcpy(ptr,      iface->name, l2);
    *ptr   = '\0';
    
    Command_appendParameter(&cmd, "label");
    Command_appendParameter(&cmd, devlabel);
  }

  Command_appendParameter(&cmd, "dev");
  Command_appendParameter(&cmd, iface->dev);

  if (!Command_exec(&cmd, true) ||
      !Command_wait(&cmd, true))
    return false;

  Command_free(&cmd);
  
  return true;
}

static bool
addVLAN(struct Interface const *iface)
{
  abort();	// TODO: implement me
}

static bool
addIndirect(struct Interface const *iface)
{
  abort();	// TODO: implement me
}

static bool
addIP(struct Interface const *iface)
{
  return invokeIpAddr(iface);
    //invokeIpLink(iface);
}

bool
Iface_add(struct Interface const *iface)
{
  if (iface->nodev)               return true;
  if (strchr(iface->dev, '.')!=0) return addVLAN(iface);
  if (!iface->direct)             return addIndirect(iface);
  return addIP(iface);
}
