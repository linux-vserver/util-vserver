// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>

#define ENSC_WRAPPERS_PREFIX	"check-unixfile: "
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

int		wrapper_exit_code = 255;

static void
showHelp(int fd, char const *cmd)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--] <file>+\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "check-unixfile " VERSION " -- execute some basic fileformat checks\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2005 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static bool
checkFile(char const *fname)
{
  int		fd   = Eopen(fname, O_RDONLY, 0);
  off_t		l    = Elseek(fd, 0, SEEK_END);
  char const *	data = 0;
  bool		res  = true;

  if (l>100*1024*1024) {
    WRITE_MSG(2, "WARNING: '");
    WRITE_STR(2, fname);
    WRITE_STR(2, "' is too large for a vserver configuration file\n");
    res = false;
  }
  else if (l>0) {
    data = mmap(0, l, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data==0) {
      perror("mmap()");
      exit(wrapper_exit_code);
    }

    if (data[l-1]!='\n') {
      WRITE_MSG(2, "WARNING: '");
      WRITE_STR(2, fname);
      WRITE_MSG(2, "' does not end on newline\n");
      res = false;
    }
    
    munmap(const_cast(char *)(data), l);
  }

  Eclose(fd);

  return res;
}

int main(int argc, char *argv[])
{
  int		idx    = 1;
  bool		ok     = true;
  bool		passed = false;

  if (argc>=2) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(1, argv[0]);
    if (strcmp(argv[1], "--version")==0) showVersion();
    if (strcmp(argv[1], "--")       ==0) ++idx;
  }

  for (; idx<argc; ++idx)
    if (checkFile(argv[idx])) passed = true;
    else                      ok     = false;

  if (ok)          return 0;
  else if (passed) return 2;
  else             return 1;
}
