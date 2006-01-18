// $Id$    --*- c++ -*--

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
#include "vserver-internal.h"
#include "getversion-internal.hc"

int
utilvserver_checkCompatVersion()
{
  static int	res=0;
  static int	v_errno;

  if (res==0) {
    res     = vc_get_version_internal(VC_CAT_COMPAT);
    v_errno = errno;
#ifdef VC_ENABLE_API_LEGACY
    if (res==-1 && (errno==ENOSYS || errno==EINVAL)) res=0;
#endif
  }

  errno = v_errno;
  return res;
}
