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

#define KERN2USR(LIMIT) \
  (((LIMIT)==CRLIM_INFINITY)  ? VC_LIM_INFINITY :		\
   ((LIMIT)==CRLIM_KEEP)      ? VC_LIM_KEEP     : (LIMIT))

#define USR2KERN(LIMIT) \
  (((LIMIT)==VC_LIM_INFINITY) ? CRLIM_INFINITY :		\
   ((LIMIT)==VC_LIM_KEEP)     ? CRLIM_KEEP     : (LIMIT))

static inline ALWAYSINLINE int
vc_get_rlimit_v11(xid_t ctx, int resource, struct vc_rlimit *lim)
{
  struct vcmd_ctx_rlimit_v0	vc_lim;
  int				rc;

  vc_lim.id        = resource;
  rc = vserver(VCMD_get_rlimit, CTX_USER2KERNEL(ctx), &vc_lim);
  lim->min  = KERN2USR(vc_lim.minimum);
  lim->soft = KERN2USR(vc_lim.softlimit);
  lim->hard = KERN2USR(vc_lim.maximum);

  return rc;
}

static inline ALWAYSINLINE int
vc_set_rlimit_v11(xid_t ctx, int resource, struct vc_rlimit const *lim)
{
  struct vcmd_ctx_rlimit_v0	vc_lim;

  vc_lim.id        = resource;
  vc_lim.minimum   = USR2KERN(lim->min);
  vc_lim.softlimit = USR2KERN(lim->soft);
  vc_lim.maximum   = USR2KERN(lim->hard);

  return vserver(VCMD_set_rlimit, CTX_USER2KERNEL(ctx), &vc_lim);
}

static inline ALWAYSINLINE int
vc_get_rlimit_mask_v11(xid_t ctx, int UNUSED tmp, struct vc_rlimit_mask *lim)
{
  struct vcmd_ctx_rlimit_v0	vc_lim;
  int				rc;

  rc = vserver(VCMD_get_rlimit_mask, CTX_USER2KERNEL(ctx), &vc_lim);

  lim->min  = vc_lim.minimum;
  lim->soft = vc_lim.softlimit;
  lim->hard = vc_lim.maximum;

  return rc;
}

#undef KERN2USR
#undef USR2KERN
