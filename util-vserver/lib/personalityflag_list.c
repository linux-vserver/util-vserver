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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.h"
#include "internal.h"


static uint_least32_t
text2personalityflag_err(char const *str,
			    size_t len, bool *failed)
{
  uint_least32_t		res = vc_text2personalityflag(str, len);
  if (res==0) *failed = true;
  return res;
}

int
vc_list2personalityflag(char const *str, size_t len,
			uint_least32_t *personality,
			struct vc_err_listparser *err)
{
  return utilvserver_listparser_uint32(str, len,
				       err ? &err->ptr : 0,
				       err ? &err->len : 0,
				       personality, 0,
				       text2personalityflag_err);
}
