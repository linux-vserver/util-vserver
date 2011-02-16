// $Id$    --*- c++ -*--

// Copyright (C) 2011 Daniel Hokka Zakrisson <daniel@hozac.com>
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

#include <sys/utsname.h>
#include <ctype.h>

int
vc_get_kernel()
{
  static int linux_ver = -1;
  if (linux_ver == -1) {
    struct utsname uts;
    char *p;
    int part, n;
    if (uname(&uts) == -1)
      return -1;
    part = 0;
    n = 0;
    linux_ver = 0;
    for (p = uts.release; *p && part < 3; p++) {
      if (*p == '.') {
	if (part < 3) {
	  linux_ver |= n << (8 * (2 - part));
	  n = 0;
	  part++;
	}
      }
      else if (isdigit(*p)) {
	n *= 10;
	n += *p - '0';
      }
      else
	break;
    }
    if (n > 0 && part < 3)
      linux_ver |= n << (8 * (2 - part));
  }
  return linux_ver;
}
