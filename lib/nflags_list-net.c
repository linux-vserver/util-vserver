// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <>
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

#include <string.h>

static uint_least64_t
vc_text2nflag_err(char const *str, size_t len, bool *failed)
{
  uint_least64_t	res = vc_text2nflag(str, len);

  if (res==0) *failed = true;
  return res;
}

int
vc_list2nflag(char const *str, size_t len,
	     struct vc_err_listparser *err,
	     struct vc_net_flags *flags)
{
  return utilvserver_listparser_uint64(str, len,
				       err ? &err->ptr : 0,
				       err ? &err->len : 0,
				       &flags->flagword, &flags->mask,
				       vc_text2nflag_err);
}
