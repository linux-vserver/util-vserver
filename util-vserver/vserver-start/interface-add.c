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
  char *			tmp = inet_ntoa(*reinterpret_cast(struct in_addr *)(&iface->addr.ipv4.ip));
  size_t			l   = strlen(tmp);
  char				addr[l + sizeof("/") + sizeof(unsigned int)*3 + 1];
  char *			ptr;
  size_t			l1 = strlen(iface->dev);
  size_t			l2 = iface->name ? strlen(iface->name) : 0;
  char				devlabel[l1 + l2 + sizeof(":")];
  bool				result = true;

  ptr    = Xmemcpy(addr, tmp, l);
  *ptr++ = '/';
  l      = utilvserver_fmt_uint(ptr, prefix);
  ptr[l] = '\0';

  Command_init(&cmd);

  size_t		idx    = 6;
  char const *		argv[] = {
    "/bin/echo",
    PROG_IP, "addr", "add",
    addr,
    "broadcast", 0,
    0, 0,	// label <name>
    0, 0,	// dev   <dev>
    0
  };

  if (iface->addr.ipv4.bcast!=0)
    argv[idx++] = inet_ntoa(*reinterpret_cast(struct in_addr *)(&iface->addr.ipv4.bcast));
  else
    argv[idx++] = "+";
  
  if (iface->name) {
    ptr    = Xmemcpy(devlabel, iface->dev,  l1);
    *ptr++ = ':';
    ptr    = Xmemcpy(ptr,      iface->name, l2);
    *ptr   = '\0';
    
    argv[idx++] = "label";
    argv[idx++] = devlabel;
  }

  argv[idx++] = "dev";
  argv[idx++] = iface->dev;  

  Command_setParams(&cmd, argv);
  if (!Command_exec(&cmd, true) ||
      !Command_wait(&cmd, true) ||
      cmd.rc!=0)
    result = false;

  Command_free(&cmd);
  
  return result;
}

static bool
addVLAN(struct Interface const UNUSED *iface)
{
  abort();	// TODO: implement me
}

static bool
addIndirect(struct Interface const UNUSED *iface)
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
