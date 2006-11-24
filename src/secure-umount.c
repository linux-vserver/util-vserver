// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "pathconfig.h"

#include "wrappers.h"
#include "util.h"

int	wrapper_exit_code = 1;

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    "[--help] [--version] <dir>\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "secure-umount " VERSION " -- secure umounting of directories\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main(int argc, char *argv[])
{
  char const *	dir;
  int		root_fd;
  int		this_fd;
  char *	umount_cmd[] = { UMOUNT_PROG, "-l", "-n", ".", 0 };

  if (argc<2) {
    WRITE_MSG(2, "Try '");
    WRITE_STR(2, argv[0]);
    WRITE_MSG(2, " --help' for more information.\n");
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "--help")==0)    showHelp(1, argv[0], 0);
  if (strcmp(argv[1], "--version")==0) showVersion();

  dir = argv[1];
  if (strcmp(dir, "--")==0 && argc>=3) dir = argv[2];

  root_fd = Eopen("/", O_RDONLY, 0);
  Echroot(".");
  Echdir(dir);
  this_fd = Eopen(".", O_RDONLY, 0);
  Efchdir(root_fd);
  Echroot(".");
  Efchdir(this_fd);

  Eclose(root_fd);
  Eclose(this_fd);

  Eexecv(umount_cmd[0], umount_cmd);
}
