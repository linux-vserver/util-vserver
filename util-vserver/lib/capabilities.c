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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "vserver.h"
#include <string.h>

#if 1
#  define DECLARECAP(X,Y) { #X, VC_##X }
#else
#  define DECLARECAP(X,Y) { #X, Y }
#endif

static struct {
    char const * const		id;
    unsigned char		bit;
} const CAP2BIT[] = {
  DECLARECAP(CAP_CHOWN,             0),
  DECLARECAP(CAP_DAC_OVERRIDE,      1),
  DECLARECAP(CAP_DAC_READ_SEARCH,   2),
  DECLARECAP(CAP_FOWNER,            3),
  DECLARECAP(CAP_FSETID,            4),
  DECLARECAP(CAP_KILL,              5),
  DECLARECAP(CAP_SETGID,            6),
  DECLARECAP(CAP_SETUID,            7),
  DECLARECAP(CAP_SETPCAP,           8),
  DECLARECAP(CAP_LINUX_IMMUTABLE,   9),
  DECLARECAP(CAP_NET_BIND_SERVICE, 10),
  DECLARECAP(CAP_NET_BROADCAST,    11),
  DECLARECAP(CAP_NET_ADMIN,        12),
  DECLARECAP(CAP_NET_RAW,          13),
  DECLARECAP(CAP_IPC_LOCK,         14),
  DECLARECAP(CAP_IPC_OWNER,        15),
  DECLARECAP(CAP_SYS_MODULE,       16),
  DECLARECAP(CAP_SYS_RAWIO,        17),
  DECLARECAP(CAP_SYS_CHROOT,       18),
  DECLARECAP(CAP_SYS_PTRACE,       19),
  DECLARECAP(CAP_SYS_PACCT,        20),
  DECLARECAP(CAP_SYS_ADMIN,        21),
  DECLARECAP(CAP_SYS_BOOT,         22),
  DECLARECAP(CAP_SYS_NICE,         23),
  DECLARECAP(CAP_SYS_RESOURCE,     24),
  DECLARECAP(CAP_SYS_TIME, 	   25),
  DECLARECAP(CAP_SYS_TTY_CONFIG,   26),
  DECLARECAP(CAP_MKNOD,            27),
  DECLARECAP(CAP_LEASE,            28),
  { "CAP_QUOTACTL",		   29 },
};
  
int
vc_text2cap(char const *str)
{
  size_t	i;
  if (strncmp(str, "CAP_", 4)==0) str += 4;

  for (i=0; i<sizeof(CAP2BIT)/sizeof(CAP2BIT[0]); ++i)
    if (strcmp(CAP2BIT[i].id+4, str)==0) return CAP2BIT[i].bit;

  return -1;
}

char const *
vc_cap2text(unsigned int bit)
{
  if ((size_t)bit>=sizeof(CAP2BIT)/sizeof(CAP2BIT[0])) return 0;
  return CAP2BIT[bit].id;
}
