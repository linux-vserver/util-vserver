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
#include "internal.h"
#include "pathconfig.h"
#include "compat-c99.h"

#include <string.h>
#include <unistd.h>

#include "getvserverbyctx-compat.hc"
#include "getvserverbyctx-v13.hc"


char *
vc_getVserverByCtx_Internal(xid_t ctx, /*@null@*/vcCfgStyle *style,
			    /*@null@*/char const *revdir,
			    bool validate_result)
{
  char *ret;
  if (vc_isSupported(vcFEATURE_MIGRATE)) {
    ret = vc_getVserverByCtx_v13(ctx, style, revdir, validate_result);
    if (ret)
      return ret;
  }
  return vc_getVserverByCtx_compat(ctx, style, revdir, validate_result);
}

char *
vc_getVserverByCtx(xid_t ctx, vcCfgStyle *style, char const *revdir)
{
  return vc_getVserverByCtx_Internal(ctx, style, revdir, true);
  
}
