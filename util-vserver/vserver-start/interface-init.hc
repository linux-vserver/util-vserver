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

#include <string.h>

static inline UNUSED void
Iface_init(struct Interface *iface)
{
  memset(&iface->addr, 0, sizeof (iface->addr));
  iface->name      = 0;
  iface->scope     = 0;
  iface->dev       = 0;
  iface->mac       = 0;
  iface->nodev     = false;
  iface->direct    = false;
  iface->up        = true;
}
