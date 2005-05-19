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
#include <errno.h>

#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_IO	1
#include <wrappers.h>

int	wrapper_exit_code = 1;

static void
showHelp(char const UNUSED *cmd)
{
  WRITE_MSG(1,
	    "Usage: chroot-cat [-i|-o|-a] [--] <file>\n"
	    "\n"
	    "Does something similarly to 'chroot . cat OP <file>' without\n"
	    "the need for a 'cat' in the chroot.\n"
	    "\n"
	    "Options:\n"
	    "    -a          ...  append to <file> (OP = '>>')\n"
	    "    -o          ...  truncate and append to <file> (OP = '>')\n"
	    "    -i          ...  use file as input (OP = '<') (default)\n"
	    "\n"
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
  int			fd;
  int			idx   = 1;
  int			fd_in;
  int			fd_out;
  
  int			xflag = O_RDONLY|O_NOCTTY;
  enum {dirIN, dirOUT}	dir   = dirIN;

  if (argc>=2) {
    if (strcmp(argv[idx], "--help")   ==0) showHelp(argv[0]);
    if (strcmp(argv[idx], "--version")==0) showVersion();
    if (strcmp(argv[idx], "-a")       ==0) { xflag = O_WRONLY|O_CREAT|O_APPEND; dir = dirOUT; ++idx; }
    if (strcmp(argv[idx], "-o")       ==0) { xflag = O_WRONLY|O_CREAT|O_TRUNC;  dir = dirOUT; ++idx; }
    if (strcmp(argv[idx], "-i")       ==0) ++idx;
    if (strcmp(argv[idx], "--")       ==0) ++idx;
  }

  if (xflag==0) {
    WRITE_MSG(2, "chroot-cat: Not mode specified; use '--help' for more information\n");
    return wrapper_exit_code;
  }
  
  if (argc<idx+1) {
    WRITE_MSG(2, "Not enough parameters; use '--help' for more information\n");
    return wrapper_exit_code;
  }

  if (argc>idx+1) {
    WRITE_MSG(2, "Too much parameters; use '--help' for more information\n");
    return wrapper_exit_code;
  }
  
  Echroot(".");
  Echdir("/");

  fd = EopenD(argv[idx], xflag, 0644);

  switch (dir) {
    case dirIN		:  fd_in = fd; fd_out =  1; break;
    default		:  fd_in =  0; fd_out = fd; break;
  }
  
  for (;;) {
    char		buf[4096];
    char const *	ptr=buf;
    ssize_t		len;

    len = Eread(fd_in, buf, sizeof(buf));
    if (len<=0) break;

    EwriteAll(fd_out, ptr, len);
  }
  Eclose(fd);

  return EXIT_SUCCESS;
}
