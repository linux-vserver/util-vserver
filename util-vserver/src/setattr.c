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

#include <lib/fmt.h>
#include <lib/vserver.h>
#include <lib/vserver-internal.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "immutable", no_argument, 0, CMD_IMMUTABLE },
  { "immulink",  no_argument, 0, CMD_IMMULINK },
  { 0,0,0,0 }
};

char const		CMDLINE_OPTIONS_SHORT[] = "Rsux";

static long	set_mask = 0;
static long	del_mask = VC_IMMUTABLE_ALL;

void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-Rsux] [--immutable] [--immulink] [--] <file>+\n\n"
	    " Options:\n"
	    "   -R  ...  recurse through directories\n"
	    "   -s  ...  set flag only; when only one of the '--immu*' options\n"
	    "            is given, do not delete the other ones\n"
	    "   -u  ...  revert operation and unset the given flags\n"
	    "   -x  ...  do not cross filesystems\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

void
showVersion()
{
  WRITE_MSG(1,
	    "setattr " VERSION " -- sets vserver specific file attributes\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

void
fixupParams(struct Arguments * args, int argc)
{
  if (optind==argc) {
    WRITE_MSG(2, "No filename given; use '--help' for more information\n");
    exit(1);
  }

  set_mask = 0;
  if (args->immutable) set_mask |= VC_IMMUTABLE_FILE_FL;
  if (args->immulink)  set_mask |= VC_IMMUTABLE_LINK_FL;
  
  if (args->do_set) del_mask = 0;
  else              del_mask = VC_IMMUTABLE_ALL;

  if (args->do_unset) {
    del_mask = set_mask;
    set_mask = 0;
  }
}

static bool
setFlags(char const *name, char const *display_name,
	 struct stat const *exp_st)
{
  int		fd = open(name, O_RDONLY);
  int		res = false;
  
  if (fd==-1) {
    perror("open()");
    return false;
  }

  if (!exp_st ||
      !checkForRace(fd, name, exp_st))
    goto err;

  if (vc_X_set_ext2flags(fd, set_mask, del_mask)==-1) {
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
  
  return setFlags(name, display_name, exp_st);
}
