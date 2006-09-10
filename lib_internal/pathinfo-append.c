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

#include "pathinfo.h"
#include "util-mem.h"

void
PathInfo_append(PathInfo       * restrict lhs,
		PathInfo const * restrict rhs,
		char *buf)
{
  char *		ptr = buf;
  char const *		rhs_ptr = rhs->d;
  size_t		rhs_len = rhs->l;

  while (lhs->l>1 && lhs->d[lhs->l-1]=='/') --lhs->l;

  if (lhs->l>0) {
    while (rhs->l>0 && *rhs_ptr=='/') {
      ++rhs_ptr;
      --rhs_len;
    }
    
    ptr = Xmemcpy(ptr, lhs->d, lhs->l);
    if (ptr[-1]!='/')
      ptr = Xmemcpy(ptr, "/", 1);
  }
//  else if (*rhs_ptr!='/')
//    ptr = Xmemcpy(ptr, "/", 1);

  ptr = Xmemcpy(ptr, rhs_ptr, rhs_len+1);

  lhs->d = buf;
  lhs->l = ptr-buf-1;
}
