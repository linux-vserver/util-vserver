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

#include "vserver.h"
#include "vserver-internal.h"
#include <fcntl.h>

#ifdef VC_ENABLE_API_FSCOMPAT
#  include "fscompat_setiattr-v11.hc"
#endif

int
vc_set_iattr_compat(char const *filename,
		    dev_t dev, ino_t ino, xid_t xid,
		    uint32_t flags, uint32_t mask)
{
#ifdef VC_ENABLE_API_FSCOMPAT
  int		ver = utilvserver_checkCompatVersion();
  if (ver==-1) return -1;
  if ((ver&0xffff)<=4)
    return vc_set_iattr_compat_v11(filename, dev, ino, xid, flags, mask);
#endif

#ifdef VC_ENABLE_API_V13
  return vc_set_iattr(dev, ino, xid, flags, mask);
#endif

  errno = ENOSYS;
  return -1;
}
