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
#include "wrappers.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int	wrapper_exit_code = 1;

int main(int argc, char *argv[])
{
  int		fd;

  if (argc!=2) {
    WRITE_MSG(2, "Usage: chroot-cat file\n");
    return EXIT_FAILURE;
  }

  Echroot(".");
  Echdir("/");

  fd = Eopen(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
  for (;;) {
    char		buf[4096];
    char const *	ptr=buf;
    ssize_t		len;

    len = Eread(0, buf, sizeof(buf));
    if (len<=0) break;

    while (len>0) {
      size_t	l = Ewrite(fd, ptr, len);
      ptr += l;
      len -= l;
    }
  }
  Eclose(fd);

  return EXIT_SUCCESS;
}
