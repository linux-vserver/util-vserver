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
#include "vserver.h"

#include "lib_internal/matchlist.h"
#include "lib_internal/unify.h"

#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <utime.h>
#include <libgen.h>

#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_DIRENT	1
#include <wrappers.h>

#define CMD_HELP		0x8000
#define CMD_VERSION		0x8001
#define CMD_MANUALLY		0x8002
#define CMD_STRICT		0x8003

struct WalkdownInfo
{
    PathInfo				state;
    struct MatchList			dst_list;
    struct MatchList			src_list;
};

struct Arguments {
    enum {mdMANUALLY, mdVSERVER}	mode;
    bool				do_dry_run;
    unsigned int			verbosity;
    bool				local_fs;
    bool				is_strict;
};

static struct WalkdownInfo		global_info;
static struct Arguments const *		global_args;

int wrapper_exit_code = 1;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "manually", no_argument,  0, CMD_MANUALLY },
  { "strict",   no_argument,  0, CMD_STRICT },
  { 0,0,0,0 }
};

typedef enum { opUNIFY, opCOPY, opDIR, opSKIP }	Operation;

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-nv] [--strict] <dst-vserver> <src-vserver>\n    or\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --manually [-nvx] [--] <dst-path> <dst-excludelist> <src-path> <src-excludelist>\n\n"
 	    "  --manually      ...  unify generic paths; excludelists must be generated\n"
	    "                       manually\n"
	    "  --strict        ...  require an existing vserver configuration for dst-vserver;\n"
	    "                       by default, a base skeleton will be created but manual\n"
	    "                       configuration wil be still needed to make the new vserver work\n"
	    "  -n              ...  do not modify anything; just show what there will be\n"
	    "                       done (in combination with '-v')\n"
	    "  -v              ...  verbose mode\n"
	    "  -x              ...  do not cross filesystems; this is valid in manual\n"
	    "                       mode only and will be ignored for vserver unification\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vcopy " VERSION " -- copies directories and vserver files\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004 Enrico Scholz\n"
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

static Operation
checkDirEntry(PathInfo const *path, struct stat const *st)
{
  struct WalkdownInfo const * const	info     = &global_info;

  if (S_ISDIR(st->st_mode)) return opDIR;
  if (S_ISLNK(st->st_mode) ||
      !S_ISREG(st->st_mode)) return opSKIP;
  
    // Check if it is in the exclude/include list of the destination vserver and
    // abort when it is not matching an allowed entry
  if (!MatchList_compare(&info->dst_list, path->d) ||
      !MatchList_compare(&info->src_list, path->d)) return opCOPY;

  return opUNIFY;
}

static bool
doit(Operation op,
     PathInfo const *dst_path,
     PathInfo const *src_path, struct stat const *exp_stat,
     PathInfo const *show_path)
{
  if (global_args->do_dry_run || global_args->verbosity>0) {
    if      (op==opUNIFY)   WRITE_MSG(1, "linking  '");
    else if (op==opCOPY)    WRITE_MSG(1, "copying  '");
    else if (op==opDIR)     WRITE_MSG(1, "creating '");
    else if (op==opSKIP)    WRITE_MSG(1, "skipping '");
    else { assert(false); abort(); }

    write(1, show_path->d, show_path->l);
    WRITE_MSG(1, "'\n");
  }

  return (global_args->do_dry_run ||
	  ( op==opSKIP) ||
	  ( op==opUNIFY && Unify_unify(src_path->d, exp_stat, dst_path->d)) ||
	  ((op==opCOPY  ||
	    op==opDIR)  && Unify_copy (src_path->d, exp_stat, dst_path->d)));
}
     
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
    perror("lstat()");
  else {
    Operation		op       = checkDirEntry(&src_path, &f_stat);
    PathInfo		dst_path = global_info.src_list.root;
    char		dst_path_buf[ENSC_PI_APPSZ(dst_path, src_path)];

    PathInfo_append(&dst_path, &src_path, dst_path_buf);
    if (!doit(op, &dst_path, &src_d_path, &f_stat, &src_path))
      perror(src_path.d);
    else if (op==opDIR) {
      res = visitDir(dirname, &f_stat);
      if (!global_args->do_dry_run &&
	  !Unify_setTime(dst_path.d, &f_stat))
	perror("utime()");
    }
    else
      res = 0;
  }

  return res;
}

#include "vcopy-init.hc"

int main(int argc, char *argv[])
{
  struct Arguments	args = {
    .mode		=  mdVSERVER,
    .do_dry_run		=  false,
    .verbosity		=  0,
    .local_fs		=  false,
  };
  uint64_t		res;

  global_args = &args;
  while (1) {
    int		c = getopt_long(argc, argv, "nvcx",
				CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_MANUALLY		:  args.mode       = mdMANUALLY; break;
      case CMD_STRICT		:  args.is_strict  = true; break;
      case 'n'			:  args.do_dry_run = true; break;
      case 'x'			:  args.local_fs   = true; break;
      case 'v'			:  ++args.verbosity;       break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (argc==optind) {
    WRITE_MSG(2, "No directory/vserver given\n");
    return EXIT_FAILURE;
  }

  switch (args.mode) {
    case mdMANUALLY	:  initModeManually(argc-optind, argv+optind); break;
    case mdVSERVER	:  initModeVserver (argc-optind, argv+optind); break;
    default		:  assert(false); return EXIT_FAILURE;
  }

  if (global_args->verbosity>3)
    WRITE_MSG(1, "Starting to traverse directories...\n");

  Echdir(global_info.src_list.root.d);
  res = visitDir("/", 0);
  
#ifndef NDEBUG
  {
    MatchList_destroy(&global_info.dst_list);
    MatchList_destroy(&global_info.src_list);
  }
#endif

  return res>0 ? 1 : 0;
}
