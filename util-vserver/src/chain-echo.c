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
#include "util.h"

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define ENSC_WRAPPERS_PREFIX	"chain-echo: "
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_IO	1
#include <wrappers.h>

int	wrapper_exit_code = 255;

static void
showHelp(char const *cmd)
{
  WRITE_MSG(1, "Usage:  ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " [--] <file> <data> <command> <args>*\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "chain-echo " VERSION " -- puts data into a file within a command-chain\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main(int argc, char *argv[])
{
  int		idx = 1;
  int		fd;

  if (argc>=2) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(argv[0]);
    if (strcmp(argv[1], "--version")==0) showVersion();
    if (strcmp(argv[1], "--")       ==0) ++idx;
  }
  
  if (argc<idx+3) {
    WRITE_MSG(2, "Not enough parameters; use '--help' for more information\n");
   return wrapper_exit_code;
  }

  if (argv[idx][0]=='\0')
    fd = 1;
  else {
    fd = Eopen(argv[idx], O_WRONLY|O_NOFOLLOW, 0600);
    Efcntl(fd, F_SETFD, FD_CLOEXEC);
  }

  if (argv[idx+1][0]!='\0')
    EwriteAll(fd, argv[idx+1], strlen(argv[idx+1]));

  Eexecv(argv[idx+2], argv+idx+2);
}
