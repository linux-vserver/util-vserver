// $Id$    --*- c -*--

// Copyright (C) 2007 Daniel Hokka Zakrisson
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
#include <lib_internal/sys_unshare.h>

static inline ALWAYSINLINE xid_t
vc_ctx_create_v21(xid_t xid, struct vc_ctx_flags *flags)
{
  struct vcmd_ctx_create data = {
	.flagword = (VC_VXF_STATE_SETUP | VC_VXF_STATE_ADMIN |
		     VC_VXF_STATE_INIT)
  };
  xid_t		res;

  if (flags)
    data.flagword = flags->flagword & flags->mask;

  res = vserver(VCMD_ctx_create, CTX_USER2KERNEL(xid), &data);
  res = CTX_KERNEL2USER(res);

  if (res != VC_NOCTX) {
    if (utilvserver_checkCompatConfig() & VC_VCI_SPACES) {
      uint32_t spaces = vc_get_space_mask() & ~(CLONE_NEWNS|CLONE_FS);
      sys_unshare(spaces);
      vc_set_namespace(VC_SAMECTX, spaces);
    }
  }

  return res;
}
