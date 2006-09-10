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
#include "internal.h"

#include "pathconfig.h"

#include "createskeleton-full.hc"
#include "createskeleton-short.hc"

#include <errno.h>

int
vc_createSkeleton(char const *id, vcCfgStyle style, int flags)
{
  if (style==vcCFG_NONE || style==vcCFG_AUTO) {
    if (strchr(id, '/')!=0) style = vcCFG_RECENT_FULL;
    else                    style = vcCFG_RECENT_SHORT;
  }
  
  switch (style) {
    case vcCFG_RECENT_SHORT	:  return vc_createSkeleton_short(id, flags);
    case vcCFG_RECENT_FULL	:  return vc_createSkeleton_full(id, 0, flags);
    default			:  ;
  }
  
  errno = EINVAL;
  return -1;
}
