// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <>
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
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  int		i;
  int		res;
  
  if (chroot(".")==-1 ||
      chdir("/")==-1) {
    perror("chroot()/chdir()");
    return EXIT_FAILURE;
  }

  res = EXIT_SUCCESS;
  for (i=1; i<argc; ++i) {
    if (unlink(argv[i])==-1) {
      WRITE_STR(2, argv[i]);
      perror("");
      res = EXIT_FAILURE;
    }
  }

  return res;
}
