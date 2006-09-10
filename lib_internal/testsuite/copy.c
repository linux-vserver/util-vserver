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

#include "lib_internal/unify.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int	wrapper_exit_code = 1;

int main(int argc, char *argv[])
{
  struct stat	st;

  if (argc<2)                 return EXIT_FAILURE;
  if (lstat(argv[1],&st)==-1) return EXIT_FAILURE;

  unlink(argv[2]);
  return Unify_copy(argv[1], &st, argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
}
