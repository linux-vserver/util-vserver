// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
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

#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>

static void
showHelp(char const *cmd)
{
  WRITE_MSG(1, "Usage:  ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " [--] <filename>\n"
	    "\n"
	    "Display value of a symbolic link on standard output.\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "readlink " VERSION " -- display value of symlink\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}


int main (int argc, char *argv[])
{
  char			buf[PATH_MAX + 2];
  int			idx = 1;
  int			len;
  
  if (argc>=2) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(argv[0]);
    if (strcmp(argv[1], "--version")==0) showVersion();
    if (strcmp(argv[1], "--")       ==0) ++idx;
  }
  if (argc<idx+1)
    WRITE_MSG(2, "No filename specified; use '--help' for more information\n");
  else if ((len=readlink(argv[idx], buf, sizeof(buf)-2))==-1)
    PERROR_Q("readlink: readlink", argv[idx]);
  else {
    buf[len++] = '\n';
    write(1, buf, len);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

