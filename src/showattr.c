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
#include <ctype.h>

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
#ifdef VC_ENABLE_API_LEGACY
  { "legacy",    no_argument, 0, CMD_LEGACY },
#endif
  { 0,0,0,0 }
};

char const		CMDLINE_OPTIONS_SHORT[] = "Radx";

void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-Radx] [--] <file>*\n\n"
	    " Options:\n"
	    "   -R  ...  recurse through directories\n"
	    "   -a  ...  display files starting with '.' also\n"
	    "   -d  ...  list directories like other files instead of listing\n"
	    "            their content\n"
	    "   -x  ...  do not cross filesystems\n\n"
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
fixupParams(struct Arguments UNUSED * args, int UNUSED argc)
{
}

static bool
getFlags(char const *name, uint32_t *flags, uint32_t *mask)
{
  xid_t		xid;
  *mask = ~0;
  
  if (vc_get_iattr(name, &xid, flags, mask)==-1) {
    perror("vc_get_iattr()");
    return false;
  }

  return true;
}

bool
handleFile(char const *name, char const *display_name)
{
  bool			res = true;
  char			buf[40];
  char			*ptr = buf;
  uint32_t		flags;
  uint32_t		mask;

  memset(buf, ' ', sizeof buf);

  if (getFlags(name, &flags, &mask)) {
      //                                     1       1       0       0
      //                              fedcba9876543210fedcba9876543210
    static char const	MARKER[33] = ".......x.....iub.............hwa";
    int		i;
    uint32_t 		used_flags = (VC_IATTR_XID|VC_IATTR_ADMIN|
				      VC_IATTR_WATCH|VC_IATTR_HIDE|
				      VC_IATTR_BARRIER|VC_IATTR_IUNLINK|
				      VC_IATTR_IMMUTABLE);

    for (i=0; i<32; ++i) {
      if (used_flags & 1) {
	if (!   (mask  & 1) ) *ptr++ = '-';
	else if (flags & 1)   *ptr++ = toupper(MARKER[31-i]);
	else                  *ptr++ = MARKER[31-i];
      }

      used_flags >>= 1;
      flags      >>= 1;
      mask       >>= 1;
    }
  }      
  else {
    memcpy(buf, "ERR   ", 7);
    res = false;
  }

  Vwrite(1, buf, 8);
  Vwrite(1, display_name, strlen(display_name));
  Vwrite(1, "\n", 1);

  return res;
}
