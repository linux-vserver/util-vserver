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
vc_set_flags_v13(xid_t xid, struct vc_ctx_flags const *flags)

{
  struct vcmd_ctx_flags_v0	k_flags;

  if (flags==0) {
    errno = EINVAL;
    return -1;
  }

  k_flags.flagword = flags->flagword;
  k_flags.mask     = flags->mask;
  
  return vserver(VCMD_set_flags, CTX_USER2KERNEL(xid), &k_flags);
}
