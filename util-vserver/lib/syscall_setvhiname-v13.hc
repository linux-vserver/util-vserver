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

static inline ALWAYSINLINE int
vc_set_vhi_name_v13(xid_t xid, vc_uts_type type, char const *val, size_t len)
{
  struct vcmd_vx_vhi_name_v0	cmd;
  int				rc;

  if (len>=sizeof(cmd.name)) {
    errno = E2BIG;
    return -1;
  }

  cmd.field = VHI_USER2KERNEL(type);
  strncpy(cmd.name, val, sizeof(cmd.name));

  rc = vserver(VCMD_vx_set_vhi_name, CTX_USER2KERNEL(xid), &cmd);
  ENSC_FIX_IOCTL(rc);

  return rc;
}
