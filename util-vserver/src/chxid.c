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

#include "fstool.h"
#include "util.h"

#include "lib/vserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { 0,0,0,0 }
};

char const		CMDLINE_OPTIONS_SHORT[] = "Rc:";

void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " -c <ctx|vserver> [-R] [--] <file>+\n\n"
	    " Options:\n"
	    "   -R  ...  recurse through directories\n"
	    "   -c  ...  assign the given context/vserver to the file(s)\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

void
showVersion()
{
  WRITE_MSG(1,
	    "chxid " VERSION " -- assigns a context to a file\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static bool
setFile(char const *name, char const * display_name, struct stat const *exp_st)
{
  bool		res = false;
  int		fd = open(name, O_RDONLY);

  if (fd==-1) {
    perror("open()");
    return false;
  }

  if (!exp_st ||
      !checkForRace(fd, name, exp_st))
    goto err;

  if (vc_X_set_filecontext(fd, global_args->ctx)==-1) {
    //perror("vc_X_set_filecontext()");
    perror(display_name);
    goto err;
  }

  res = true;

  err:
  close(fd);
  return res;
}

bool
handleFile(char const *name, char const * display_name,
	   struct stat const *exp_st)
{
  if (S_ISLNK(exp_st->st_mode)) return true;
  
  return setFile(name, display_name, exp_st);
}

void
fixupParams(struct Arguments UNUSED *args, int argc)
{
  if (optind==argc) {
    WRITE_MSG(2, "No filename given; use '--help' for more information\n");
    exit(1);
  }

  if (args->ctx_str==0) {
    WRITE_MSG(2, "No context given; use '--help' for more information\n");
    exit(1);
  }

  args->ctx = resolveCtx(args->ctx_str);
}
