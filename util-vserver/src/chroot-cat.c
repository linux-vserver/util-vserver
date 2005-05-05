// $Id$    --*- c -*--

// Copyright (C) 2003,2004,2005 Enrico Scholz <>
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
#include <fcntl.h>

#define ENSC_WRAPPERS_UNISTD 1
#define ENSC_WRAPPERS_FCNTL  1
#include <wrappers.h>

int	wrapper_exit_code = 1;

static void
showHelp(char const UNUSED *cmd)
{
  WRITE_MSG(1,
	    "Usage: chroot-cat [-a] [--] <file>\n"
	    "\n"
	    "Does something similarly to 'chroot . cat > <file>' without\n"
	    "the need for a 'cat' in the chroot.\n"
	    "\n"
	    "Options:\n"
	    "    -a          ...  append instead of truncate (do '>> <file>'\n"
	    "                     instead of '> <file>)\n\n"
 	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "chroot-cat " VERSION " -- cat stdin into a chrooted file\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004,2005 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}


int main(int argc, char *argv[])
{
  int		fd;
  int		idx = 1;
  bool		do_append = false;

  if (argc>=2) {
    if (strcmp(argv[idx], "--help")   ==0) showHelp(argv[0]);
    if (strcmp(argv[idx], "--version")==0) showVersion();
    if (strcmp(argv[idx], "-a")       ==0) { do_append = true; ++idx; }
    if (strcmp(argv[idx], "--")       ==0) ++idx;
  }
  
  if (argc!=idx+1) {
    WRITE_MSG(2, "Not enough parameters; use '--help' for more information\n");
    return wrapper_exit_code;
  }

  Echroot(".");
  Echdir("/");

  fd = EopenD(argv[idx], O_WRONLY|O_CREAT| (do_append ? O_APPEND : O_TRUNC), 0644);
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
