// $Id$    --*- c -*--

// Copyright (C) 2007 Daniel Hokka Zakrisson
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
#include "vserver.h"

#include "lib_internal/pathinfo.h"
#include "lib_internal/unify.h"

#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <utime.h>
#include <libgen.h>
#include <sys/param.h>

#define ENSC_WRAPPERS_PREFIX	"vclone: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_DIRENT	1
#include <wrappers.h>

#define CMD_HELP		0x8000
#define CMD_VERSION		0x8001
#define CMD_SOURCE		0x8002
#define CMD_DEST		0x8003

struct WalkdownInfo
{
    PathInfo	state;
    PathInfo	src;
    PathInfo	dst;
};

struct Arguments {
    unsigned int	verbosity;
};

static struct WalkdownInfo		global_info;
static struct Arguments const *		global_args;

int wrapper_exit_code = 1;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,       0, CMD_HELP },
  { "version",  no_argument,       0, CMD_VERSION },
  { "source",   required_argument, 0, CMD_SOURCE },
  { "dest",     required_argument, 0, CMD_DEST },
  { 0,0,0,0 }
};


static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --source <source> --dest <destination>\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vclone " VERSION " -- clones a guest\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2007 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int Global_getVerbosity() {
  return global_args->verbosity;
}

bool Global_doRenew() {
  return true;
}

#include "vserver-visitdir.hc"

static uint64_t
visitDirEntry(struct dirent const *ent)
{
  char const *			dirname  = ent->d_name;
  if (isDotfile(dirname)) return 0;

  uint64_t			res      = 1;
  PathInfo			src_path = global_info.state;
  PathInfo			src_d_path = {
    .d = dirname,
    .l = strlen(dirname)
  };
  char				path_buf[ENSC_PI_APPSZ(src_path, src_d_path)];
  struct stat			f_stat = { .st_dev = 0 };

  PathInfo_append(&src_path, &src_d_path, path_buf);

  
  if (lstat(dirname, &f_stat)==-1)
    perror(ENSC_WRAPPERS_PREFIX "lstat()");
  else {
    PathInfo		dst_path = global_info.dst;
    char		dst_path_buf[ENSC_PI_APPSZ(dst_path, src_path)];

    PathInfo_append(&dst_path, &src_path, dst_path_buf);
    if (S_ISREG(f_stat.st_mode) && Unify_isIUnlinkable(src_d_path.d) == unifyBUSY) {
      Elink(src_d_path.d, dst_path.d);
      res = 0;
    }
    else {
      if (!Unify_copy(src_d_path.d, &f_stat, dst_path.d)) {
	perror(ENSC_WRAPPERS_PREFIX "Unify_copy()");
	exit(wrapper_exit_code);
      }
      if (S_ISDIR(f_stat.st_mode))
        res = visitDir(dirname, &f_stat);
      else
	res = 0;
    }
  }

  return res;
}

int main(int argc, char *argv[])
{
  struct Arguments	args = {
    .verbosity		=  0,
  };
  uint64_t		res;

  global_args = &args;
  while (1) {
    int		c = getopt_long(argc, argv, "+",
				CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_SOURCE	:  ENSC_PI_SETSTR(global_info.src, optarg); break;
      case CMD_DEST	:  ENSC_PI_SETSTR(global_info.dst, optarg); break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (global_args->verbosity>3)
    WRITE_MSG(1, "Starting to traverse directories...\n");

  Echdir(global_info.src.d);
  res = visitDir("/", 0);
  
  return res>0 ? 1 : 0;
}
