// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <ensc@delenn.intern.sigma-chemnitz.de>
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
#include <lib_internal/util-dimof.h>
#include <linux/personality.h>

#define DECL(VAL) { #VAL, sizeof(#VAL)-1, (VAL) }

static struct Mapping_uint32 const VALUES[] = {
#if HAVE_DECL_MMAP_PAGE_ZERO
  DECL(MMAP_PAGE_ZERO),
#endif

#if HAVE_DECL_ADDR_LIMIT_32BIT  
  DECL(ADDR_LIMIT_32BIT),
#endif

#if HAVE_DECL_SHORT_INODE
  DECL(SHORT_INODE),
#endif

#if HAVE_DECL_WHOLE_SECONDS
  DECL(WHOLE_SECONDS),
#endif

#if HAVE_DECL_STICKY_TIMEOUTS
  DECL(STICKY_TIMEOUTS),
#endif

#if HAVE_DECL_ADDR_LIMIT_3GB
  DECL(ADDR_LIMIT_3GB),
#endif
};

uint_least32_t
vc_text2personalityflag(char const *str, size_t len)
{
  ssize_t	idx = utilvserver_value2text_uint32(str, len,
						    VALUES, DIM_OF(VALUES));

  if (idx==-1) return 0;
  else         return VALUES[idx].val;
}

char const *
vc_lopersonality2text(uint_least32_t *val)
{
  ssize_t	idx = utilvserver_text2value_uint32(val, VALUES,
						    DIM_OF(VALUES));

  if (idx==-1) return 0;
  else         return VALUES[idx].id;
}

  
