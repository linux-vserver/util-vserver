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

#include <lib_internal/filecfg.h>

char const data[] = "\
line 1\n\
#line 2\n\
\n\
line 4\n\
 \n\
line 6\n\
 \tline 7\n\
line 8\t  \n\
";

static bool
test(void *x_p, char const *str, size_t len)
{
  size_t	*x = x_p;

  if (strncmp(str, "line ", 5)!=0) abort();
  if (!isdigit(str[len-1]))        abort();
  ++*x;
  return true;
}

int main()
{
  size_t	l = 1;
  FileCfg_iterateOverMultiLine(data, test, &l);
  if (l!=6) abort();
}
