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
#include "lib_internal/matchlist.h"

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
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#define CMD_HELP		0x8000
#define CMD_VERSION		0x8001
#define CMD_XID			0x8002

struct WalkdownInfo
{
    PathInfo		state;
    PathInfo		src;
    PathInfo		dst;
    struct MatchList	excludes;
};

struct Arguments {
    unsigned int	verbosity;
    xid_t		xid;
    const char *	exclude_list;
};

static struct WalkdownInfo		global_info;
static struct Arguments const *		global_args;

int wrapper_exit_code = 1;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",         no_argument,       0, CMD_HELP },
  { "version",      no_argument,       0, CMD_VERSION },
  { "xid",          required_argument, 0, CMD_XID },
  { "exclude-from", required_argument, 0, 'X' },
  { 0,0,0,0 }
};


static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--xid <xid>] [--exclude-from <exclude-list>]\n"
	    "         <source> <absolute path to destination>\n\n"
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

static bool
handleDirEntry(const PathInfo *src_path, const PathInfo *basename,
	      bool *is_dir, struct stat *st)
{
  bool res = false;

  *is_dir = false;

  if (lstat(basename->d, st)==-1)
    PERROR_Q(ENSC_WRAPPERS_PREFIX "lstat", src_path->d);
  else {
    PathInfo		dst_path = global_info.dst;
    char		dst_path_buf[ENSC_PI_APPSZ(dst_path, *src_path)];
    struct stat		dst_st;

    if (S_ISDIR(st->st_mode))
      *is_dir = true;

    if (MatchList_compare(&global_info.excludes, src_path->d) != stINCLUDE) {
      if (Global_getVerbosity() > 1) {
	WRITE_MSG(1, "  skipping '");
	Vwrite(1, src_path->d, src_path->l);
	WRITE_MSG(1, "' (excluded)\n");
      }
      return true;
    }

    PathInfo_append(&dst_path, src_path, dst_path_buf);

    /* skip files that already exist */
    if (lstat(dst_path.d, &dst_st)!=-1) {
      if (Global_getVerbosity() > 1) {
	WRITE_MSG(1, "  skipping '");
	Vwrite(1, src_path->d, src_path->l);
	WRITE_MSG(1, "' (exists in destination)\n");
      }
      res = true;
    }
    else {
      /* create directory that might have been skipped */
      if (global_info.excludes.skip_depth > 0) {
	if (Global_getVerbosity() > 4) {
	  WRITE_MSG(1, "  creating directories for '");
	  Vwrite(1, dst_path.d, dst_path.l);
	  WRITE_MSG(1, "'\n");
	}
	if (mkdirRecursive(dst_path.d) == -1)
	  PERROR_Q(ENSC_WRAPPERS_PREFIX "mkdirRecursive", dst_path.d);
      }

      /* already unified file */
      if (S_ISREG(st->st_mode) && Unify_isIUnlinkable(basename->d) == unifyBUSY) {
	if (Global_getVerbosity() > 2) {
	  WRITE_MSG(1, "  linking unified file '");
	  Vwrite(1, src_path->d, src_path->l);
	  WRITE_MSG(1, "'\n");
	}
	Elink(basename->d, dst_path.d);
	res = true;
      }
      /* something we have to copy */
      else {
	if (Global_getVerbosity() > 2) {
	  WRITE_MSG(1, "  copying non-unified file '");
	  Vwrite(1, src_path->d, src_path->l);
	  WRITE_MSG(1, "'\n");
	}
	if (!Unify_copy(basename->d, st, dst_path.d))
	  PERROR_Q(ENSC_WRAPPERS_PREFIX "Unify_copy", dst_path.d);
	else if (!S_ISSOCK(st->st_mode) &&
		 global_args->xid != VC_NOCTX &&
		 vc_set_iattr(dst_path.d, global_args->xid, 0, VC_IATTR_XID) == -1 &&
		 errno != EINVAL)
	  PERROR_Q(ENSC_WRAPPERS_PREFIX "vc_set_iattr", dst_path.d);
	else
	  res = true;
      }
    }
  }

  return res;
}

/* returns 1 on error, 0 on success */
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
  bool				is_dir;

  PathInfo_append(&src_path, &src_d_path, path_buf);

  if (handleDirEntry(&src_path, &src_d_path, &is_dir, &f_stat))
    res = 0;

  if (is_dir) {
    if (res || global_info.excludes.skip_depth > 0)
      global_info.excludes.skip_depth++;
    res = res + visitDir(dirname, &f_stat);
    if (global_info.excludes.skip_depth > 0)
      global_info.excludes.skip_depth--;
  }

  return res;
}

int main(int argc, char *argv[])
{
  struct Arguments	args = {
    .verbosity		=  0,
    .xid		= VC_NOCTX,
    .exclude_list	= NULL,
  };
  uint64_t		res;
  int			num_args;

  global_args = &args;
  while (1) {
    int		c = getopt_long(argc, argv, "+vX:",
				CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case 'v'		:  args.verbosity++; break;
      case 'X'		:  args.exclude_list = optarg; break;
      case CMD_XID	:  args.xid = Evc_xidopt2xid(optarg,true); break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  num_args = argc - optind;
  if (num_args < 1) {
    WRITE_MSG(2, "Source is missing; try '");
    WRITE_STR(2, argv[0]);
    WRITE_MSG(2, " --help' for more information.\n");
    return EXIT_FAILURE;
  }
  else if (num_args < 2) {
    WRITE_MSG(2, "Destination is missing; try '");
    WRITE_STR(2, argv[0]);
    WRITE_MSG(2, " --help' for more information.\n");
    return EXIT_FAILURE;
  }
  else if (num_args > 2) {
    WRITE_MSG(2, "Too many arguments; try '");
    WRITE_STR(2, argv[0]);
    WRITE_MSG(2, " --help' for more information.\n");
    return EXIT_FAILURE;
  }
  else if (*argv[optind+1] != '/') {
    WRITE_MSG(2, "The destination must be an absolute path; try '");
    WRITE_STR(2, argv[0]);
    WRITE_MSG(2, " --help' for more information.\n");
    return EXIT_FAILURE;
  }
  ENSC_PI_SETSTR(global_info.src, argv[optind]);
  ENSC_PI_SETSTR(global_info.dst, argv[optind+1]);

  if (global_args->exclude_list)
    MatchList_initManually(&global_info.excludes, 0, strdup(argv[optind]),
			   global_args->exclude_list);
  else
    MatchList_init(&global_info.excludes, argv[optind], 0);

  if (global_args->verbosity>3)
    WRITE_MSG(1, "Starting to traverse directories...\n");

  Echdir(global_info.src.d);
  res = visitDir("/", 0);

  MatchList_destroy(&global_info.excludes);
  
  return res>0 ? 1 : 0;
}
