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

#include "util.h"
#include <stdlib.h>

static inline void
showHelp(int fd, int exit_code)
{
  WRITE_MSG(fd,
	    "Usage:  mask2prefix <mask>\n");
  exit(exit_code);
}

int main(int argc, char *argv[])
{
  char		*err_ptr, *ptr;
  int		len = 0;
  size_t	i;
  
  if (argc!=2) showHelp(2,255);

  ptr = argv[1];
  for (i=0; i<4; ++i) {
    unsigned int	val = strtol(ptr, &err_ptr, 10);

    switch (*err_ptr) {
      case '.'	:
      case '\0'	:  break;
      default	:
	WRITE_MSG(2, "Invalid mask specified\n");
	return 255;
    }
    
    if (val>=0x100) {
      WRITE_MSG(2, "Invalid mask specified\n");
      return 255;
    }

    while (val&0x80) {
      ++len;
      val <<= 1;
    }

    if (val!=0xff00) break;

    ptr = err_ptr+1;
  }

  return len;
}
