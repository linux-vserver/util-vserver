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
#include "compat.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

size_t
utilvserver_uint2str(char *buf, size_t len, unsigned int val, unsigned char base)
{
  char			*ptr = buf+len-1;
  register size_t	res;
  if (base>=36 || len==0) return 0;

  *ptr = '\0';
  while (ptr>buf) {
    unsigned char	digit = val%base;
    
    --ptr;
    *ptr = (digit<10 ? '0'+digit :
	    digit<36 ? 'a'+digit-10 :
	    (assert(false),'?'));

    val /= base;
    if (val==0) break;
  }

  assert(ptr>=buf && ptr<=buf+len-1);
	 
  res = buf+len-ptr;
  memmove(buf, ptr, res);

  return res-1;
}
