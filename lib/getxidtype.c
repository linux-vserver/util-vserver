// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#include "internal.h"

vcXidType
vc_getXIDType(xid_t xid)
{
  static xid_t	MIN_D_CONTEXT = 0;
  const xid_t	MAX_S_CONTEXT = 65535;
  if (MIN_D_CONTEXT == 0 && (utilvserver_checkCompatConfig() & VC_VCI_NO_DYNAMIC) == 0)
    MIN_D_CONTEXT = 49152;
  else
    MIN_D_CONTEXT = MAX_S_CONTEXT;

  if (xid==0)					return vcTYPE_MAIN;
  if (xid==1)					return vcTYPE_WATCH;
  if (xid>1              && xid<MIN_D_CONTEXT)	return vcTYPE_STATIC;
  if (xid>=MIN_D_CONTEXT && xid<MAX_S_CONTEXT)	return vcTYPE_DYNAMIC;
  return vcTYPE_INVALID;
}
