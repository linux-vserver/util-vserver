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

#include <lib/vserver.h>
#include <lib/vserver-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { 0,0,0,0 }
};

char const		CMDLINE_OPTIONS_SHORT[] = "Radnx";

void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-Radnx] [--] <file>*\n\n"
	    " Options:\n"
	    "   -R  ...  recurse through directories\n"
	    "   -a  ...  display files starting with '.' also\n"
	    "   -d  ...  list directories like other files instead of\n"
	    "            listing their content\n"
	    "   -n  ...  do not try to do xid -> vserver-name mapping\n"
	    "   -x  ...  do not cross filesystems\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

void
showVersion()
{
  WRITE_MSG(1,
	    "lsxid " VERSION " -- shows the context which is associated to a file\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

void
fixupParams(struct Arguments UNUSED * args, int UNUSED argc)
{
}

static xid_t
getFileContext(char const *name, struct stat const *exp_st)
{
  int		fd = open(name, O_RDONLY);
  xid_t		res;
  
  if (fd==-1) {
    perror("open()");
    return VC_NOCTX;
  }

  if (exp_st)
    checkForRace(fd, name, exp_st);

  res = vc_X_get_filecontext(fd);
  if (res==VC_NOCTX)
    perror("vc_X_get_filecontext()");
  
  close(fd);

  return res;
}

bool
handleFile(char const *name, char const *display_name,
	   struct stat const *exp_st)
{
  xid_t		ctx = 0;
  char		buf[MAX(sizeof(ctx)*3+1, 20)];
  bool		need_write = true;

  memset(buf, ' ', sizeof buf);

  if (S_ISLNK(exp_st->st_mode)) {
    memcpy(buf, "-------", 7);
    write(1, buf, sizeof buf);
    need_write = false;
  }
  else {
    ctx = getFileContext(name, exp_st);
  
    if (ctx==VC_NOCTX) {
      memcpy(buf, "!!ERR!!", 7);
      write(1, buf, sizeof buf);
      need_write = false;
    }
    else if (global_args->do_mapping) {
      char const *	vname = vc_getVserverByCtx(ctx, 0,0);
      if (!vname) buf[0] = '\0';
      else {
	size_t		l = strlen(vname);
	if (l<sizeof(buf)) write(1, buf, sizeof(buf)-l);
	write(1, vname, l);

	free((char *)vname);
	need_write = false;
      }
    }
  }

  if (need_write) {
    size_t	l = utilvserver_fmt_ulong(buf, ctx);
    if (l<sizeof(buf)) write(1, buf+l, sizeof(buf)-l);
    write(1, buf, l);
  }

  write(1, "  ", 2);
  write(1, display_name, strlen(display_name));
  write(1, "\n", 1);

  return ctx!=VC_NOCTX;
}
