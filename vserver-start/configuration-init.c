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

#include "configuration.h"
#include "interface.h"
void
Cfg_init(struct Configuration *cfg)
{
  struct vc_ctx_caps const	caps = {
    .bcaps = 0,
    .bmask = 0,
    .ccaps = 0,
    .cmask = 0
  };

  struct vc_ctx_flags const	flags = {
    .flagword = 0,
    .mask     = 0
  };
  
  Vector_init(&cfg->interfaces, sizeof(struct Interface));
  cfg->vdir      = 0;
  cfg->xid       = VC_DYNAMIC_XID;
  cfg->broadcast = 0;
  cfg->ctx_caps  = caps;
  cfg->ctx_flags = flags;
}
