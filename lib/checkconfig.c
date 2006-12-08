// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// Copyright (C) 2006 Daniel Hokka Zakrisson
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

uint_least32_t
utilvserver_checkCompatConfig()
{
#ifdef VC_ENABLE_API_V21
  static uint32_t	res=0;
  static int		v_errno;

  if (res==0) {
    res     = vc_get_vci();
    v_errno = errno;
    if (res==(uint32_t)-1 && (errno==ENOSYS || errno==EINVAL)) res=0;
  }

  errno = v_errno;
  return res;
#else
  return 0;
#endif
}
