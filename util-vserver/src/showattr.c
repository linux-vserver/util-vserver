// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on showattr.cc by Jacques Gelinas
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
#ifdef VC_ENABLE_API_LEGACY
  { "legacy",    no_argument, 0, CMD_LEGACY },
#endif
  { 0,0,0,0 }
};

char const		CMDLINE_OPTIONS_SHORT[] = "Rad";

void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-Rad] [--] <file>*\n\n"
	    " Options:\n"
	    "   -R  ...  recurse through directories\n"
	    "   -a  ...  display files starting with '.' also\n"
	    "   -d  ...  list directories like other files instead of listing\n"
	    "            their content\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

void
showVersion()
{
  WRITE_MSG(1,
	    "showattr " VERSION " -- shows vserver specific file attributes\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

void
checkParams(struct Arguments const UNUSED * args, int UNUSED argc)
{
}

static bool
getFlags(char const *name, struct stat const *exp_st, long *flags)
{
  int		fd = open(name, O_RDONLY);
  int		rc;
  
  if (fd==-1) {
    perror("open()");
    return false;
  }

  if (exp_st)
    checkForRace(fd, name, exp_st);

  rc = vc_X_get_ext2flags(fd, flags);
  *flags &= VC_IMMUTABLE_ALL;

  if (rc==-1)
    perror("vc_X_get_ext2flags()");

  close(fd);
  return rc!=-1;
}

static void
writePadded(long num)
{
  char		buf[sizeof(num)*2+1];
  size_t	l = utilvserver_fmt_xulong(buf, num);

  if (l<8) write(1, "00000000", 8-l);
  write(1, buf, l);
}

#ifdef VC_ENABLE_API_LEGACY
static bool
handleFileLegacy(char const *name, char const *display_name,
		 struct stat const *exp_st)
{
  long		flags;

  if (S_ISLNK(exp_st->st_mode)) {
    write(1, display_name, strlen(display_name));
    write(1, "  -\n", 2);
    return true;
  }

  if (!getFlags(name, exp_st, &flags)) {
    perror(display_name);
    return false;
  }

  write(1, display_name, strlen(display_name));
  write(1, "\t", 1);
  writePadded(flags);
  write(1, "\n", 1);

  return true;
}
#endif

bool
handleFile(char const *name, char const *display_name,
	   struct stat const *exp_st)
{
  bool		res = true;
#ifdef VC_ENABLE_API_LEGACY
  if (global_args->is_legacy)
    return handleFileLegacy(name, display_name, exp_st);
#endif
  
  if (S_ISLNK(exp_st->st_mode)) {
    write(1, "--------", 8);
  }
  else {
    long		flags;

    if (getFlags(name, exp_st, &flags))
      writePadded(flags);
    else {
      write(1, "ERR     ", 8);
      res = false;
    }
  }

  write(1, "  ", 2);
  write(1, display_name, strlen(display_name));
  write(1, "\n", 1);

  return res;
}
