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

#define ENSC_TESTSUITE
#define main	Xmain
#  include "../vhashify.c"
#undef main

#include "lib_internal/coreassert.h"

int main(int UNUSED argc, char *argv[])
{
  int		fd = open(argv[1], O_NOFOLLOW|O_NONBLOCK|O_RDONLY|O_NOCTTY);
  struct stat	st;
  HashPath 	d_path;
  off_t		size;

  global_info.hash_conf.method = hashFunctionFind(argv[2]);
  
  assert(hashFunctionContextInit(&global_info.hash_context,
				 global_info.hash_conf.method)!=-1);

  assert(fstat(fd, &st)!=-1);

    // set members of st to defined values so that the hash (which is
    // influenced by them) is predictable
  size = st.st_size;
  memset(&st, 0, sizeof st);
  st.st_size = size;
  
  assert(calculateHashFromFD(fd, d_path, &st));

  Vwrite(1, d_path, strlen(d_path));
  Vwrite(1, "\n", 1);
  
  hashFunctionContextFree(&global_info.hash_context);
  
  return 0;
}
