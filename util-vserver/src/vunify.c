// $Id$    --*- c -*--

// Copyright (C) 2003,2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "vunify.h"
#include "util.h"

#include "lib_internal/unify.h"
#include "lib_internal/matchlist.h"
#include "lib_internal/util-dotfile.h"
#include "lib_internal/util-safechdir.h"
#include <lib/vserver.h>

#include <getopt.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <wait.h>
#include <fcntl.h>
#include <assert.h>

#define ENSC_WRAPPERS_IO	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_DIRENT	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_STDLIB	1
#include <wrappers.h>

int	wrapper_exit_code = 1;


#define CMD_HELP		0x8000
#define CMD_VERSION		0x8001
#define CMD_MANUALLY		0x8002

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "manually", no_argument,  0, CMD_MANUALLY },
  { 0,0,0,0 }
};

static struct WalkdownInfo		global_info;
static struct SkipReason		skip_reason;
static struct Arguments const *		global_args;

int Global_getVerbosity() {
  return global_args->verbosity;
}

bool Global_doRenew() {
  return global_args->do_renew;
}

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-Rnv] <vserver>\n    or\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --manually [-Rnvx] [--] <path> <excludelist> [<path> <excludelist>]+\n\n"
 	    "  --manually      ...  unify generic paths; excludelists must be generated\n"
	    "                       manually\n"
	    "  -R              ...  revert operation; deunify files\n"
	    "  -n              ...  do not modify anything; just show what there will be\n"
	    "                       done (in combination with '-v')\n"
	    "  -v              ...  verbose mode\n"
	    "  -x              ...  do not cross filesystems; this is valid in manual\n"
	    "                       mode only and will be ignored for vserver unification\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
#if 0	    
	    "  -C              ...  use cached excludelists; usually they will be\n"
	    "                       regenerated after package installation to reflect e.g.\n"
	    "                       added/removed configuration files\n\n"
#endif	    
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vunify " VERSION " -- unifies vservers and/or directories\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

// Returns 'false' iff one of the files is not existing, or of the files are different/not unifyable
static bool
checkFstat(struct MatchList const * const mlist,
	   PathInfo const * const  basename,
	   PathInfo const * const  path,
	   struct stat const ** const dst_fstat, struct stat * const dst_fstat_buf,
	   struct stat * const src_fstat)
{
  assert(basename->d[0] != '/');

  if (*dst_fstat==0) {
    // local file does not exist... strange
    // TODO: message
    skip_reason.r = rsFSTAT;
    if (lstat(basename->d, dst_fstat_buf)==-1) return false;
    *dst_fstat = dst_fstat_buf;
  }

  assert(*dst_fstat!=0);
  
  {
    PathInfo		src_path = mlist->root;
    char		src_path_buf[ENSC_PI_APPSZ(src_path, *path)];

    PathInfo_append(&src_path, path, src_path_buf);

    // source file does not exist
    skip_reason.r = rsNOEXISTS;
    if (lstat(src_path.d, src_fstat)==-1) return false;

    // these are directories; this succeeds everytime
    if (S_ISDIR((*dst_fstat)->st_mode) && S_ISDIR(src_fstat->st_mode)) return true;

    // both files are different, so return false
    skip_reason.r = rsDIFFERENT;
    if ((!global_args->do_revert && !Unify_isUnifyable(*dst_fstat, src_fstat)) ||
	( global_args->do_revert && !Unify_isUnified  (*dst_fstat, src_fstat)))
      return false;
  }

  // these are the same files
  return true;
}

static struct MatchList const *
checkDirEntry(PathInfo const *path,
	      PathInfo const *d_path, bool *is_dir,
	      struct stat *src_stat, struct stat *dst_stat)
{
  struct WalkdownInfo const * const	info     = &global_info;
  struct MatchList const *		mlist;
  struct stat const *			cache_stat;

  // Check if it is in the exclude/include list of the destination vserver and
  // abort when it is not matching an allowed entry
  skip_reason.r      = rsEXCL_DST;
  skip_reason.d.list = &info->dst_list;
  if (!MatchList_compare(&info->dst_list, path->d)) return 0;

  // Now, go through the reference vservers and do the lightweigt list-check
  // first and compare then the fstat's.
  for (mlist=info->src_lists.v; mlist<info->src_lists.v+info->src_lists.l; ++mlist) {
    cache_stat = 0;
    skip_reason.r      = rsEXCL_SRC;
    skip_reason.d.list = mlist;
    if (MatchList_compare(mlist, path->d) &&
	checkFstat(mlist, d_path, path, &cache_stat, dst_stat, src_stat)) {

      // Failed the check or is it a symlink which can not be handled
      if (cache_stat==0) return 0;

      skip_reason.r = rsSYMLINK;
      if (S_ISLNK(dst_stat->st_mode)) return 0;

      skip_reason.r = rsSPECIAL;
      if (!S_ISREG(dst_stat->st_mode) &&
	  !S_ISDIR(dst_stat->st_mode)) return 0;
      
      *is_dir = S_ISDIR(dst_stat->st_mode);
      return mlist;
    }
    else if (cache_stat!=0 && !global_args->do_revert &&
	     skip_reason.r == rsDIFFERENT &&
	     Unify_isUnified(cache_stat, src_stat)) {
      skip_reason.r      = rsUNIFIED;
      skip_reason.d.list = mlist;
      return 0;
    }
  }

  // No luck...
  return 0;
}

static bool
updateSkipDepth(PathInfo const *path, bool walk_down)
{
  struct WalkdownInfo const * const	info   = &global_info;
  struct MatchList *			mlist;
  bool					result = false;

  for (mlist=info->src_lists.v; mlist<info->src_lists.v+info->src_lists.l; ++mlist) {
    // The easy way... this path is being skipped already
    if (mlist->skip_depth>0) {
      if (walk_down) ++mlist->skip_depth;
      else           --mlist->skip_depth;
      continue;
    }
    else if (walk_down) {
      PathInfo		src_path = mlist->root;
      char		src_path_buf[ENSC_PI_APPSZ(src_path, *path)];
      struct stat	src_fstat;

      PathInfo_append(&src_path, path, src_path_buf);

      // when the file/dir exist, we have do go deeper.
      // else skip it in deeper runs for *this* matchlist
      if (lstat(src_path.d, &src_fstat)!=-1) result = true;
      else                                   ++mlist->skip_depth;
    }
    else {
      // TODO: warning
    }
  }

  return result;
}

static bool
doit(struct MatchList const *mlist,
     PathInfo const *src_path, struct stat const *src_stat,
     char const *dst_path,     struct stat const UNUSED *dst_stat)
{
  PathInfo	path = mlist->root;
  char		path_buf[ENSC_PI_APPSZ(path, *src_path)];

  if (global_args->do_dry_run || Global_getVerbosity()>=2) {
    if (global_args->do_revert) WRITE_MSG(1, "deunifying '");
    else                        WRITE_MSG(1, "unifying   '");

    write(1, src_path->d, src_path->l);
    WRITE_MSG(1, "'");

    if (Global_getVerbosity()>=4) {
      WRITE_MSG(1, " (from ");
      if (Global_getVerbosity()==4 && mlist->id.d)
	write(1, mlist->id.d, mlist->id.l);
      else
	write(1, mlist->root.d, mlist->root.l);
      WRITE_MSG(1, ")");
    }
    WRITE_MSG(1, "\n");
  }
  
  PathInfo_append(&path, src_path, path_buf);
  return (global_args->do_dry_run ||
	  (!global_args->do_revert && Unify_unify  (path.d, src_stat, dst_path)) ||
	  ( global_args->do_revert && Unify_deUnify(dst_path)));
}


static void
printSkipReason()
{
  WRITE_MSG(1, " (");
  switch (skip_reason.r) {
    case rsDOTFILE	:  WRITE_MSG(1, "dotfile"); break;
    case rsEXCL_DST	:
    case rsEXCL_SRC	:
      WRITE_MSG(1, "excluded by ");
      MatchList_printId(skip_reason.d.list, 1);
      break;
    case rsFSTAT	:  WRITE_MSG(1, "fstat error"); break;
    case rsNOEXISTS	:  WRITE_MSG(1, "does not exists in refserver(s)"); break;
    case rsSYMLINK	:  WRITE_MSG(1, "symlink"); break;
    case rsSPECIAL	:  WRITE_MSG(1, "non regular file"); break;
    case rsUNIFIED	:  WRITE_MSG(1, "already unified"); break;
    case rsDIFFERENT	:  WRITE_MSG(1, "different"); break;
    default		:  assert(false); abort();
  }
  WRITE_MSG(1, ")");
}

#include "vserver-visitdir.hc"

static uint64_t
visitDirEntry(struct dirent const *ent)
{
  bool				is_dir;
  struct MatchList const *	match;
  struct stat			f_stat = { .st_dev = 0 };
  char const *			dirname  = ent->d_name;
  PathInfo			path     = global_info.state;
  PathInfo			d_path = {
    .d = dirname,
    .l = strlen(dirname)
  };
  char				path_buf[ENSC_PI_APPSZ(path, d_path)];
  bool				is_dotfile;
  struct stat			src_stat;
  uint64_t			res = 1;

  PathInfo_append(&path, &d_path, path_buf);

  is_dotfile    = isDotfile(dirname);
  skip_reason.r = rsDOTFILE;

  if (is_dotfile ||
      (match=checkDirEntry(&path, &d_path, &is_dir, &src_stat, &f_stat))==0) {
    bool	is_link = is_dotfile ? false : S_ISLNK(f_stat.st_mode);
    
    if (Global_getVerbosity()>=1 &&
	(Global_getVerbosity()>=3 || skip_reason.r!=rsUNIFIED) &&
	((!is_dotfile && !is_link) ||
	 (Global_getVerbosity()>=6 && is_dotfile) ||
	 (Global_getVerbosity()>=6 && is_link)) ) {
      WRITE_MSG(1, "  skipping '");
      write(1, path.d, path.l);
      WRITE_MSG(1, "'");
      if (Global_getVerbosity()>=2) printSkipReason();
      WRITE_MSG(1, "\n");
    }
    return 0;
  }

  if (is_dir) {
    if (updateSkipDepth(&path, true)) {
      res = visitDir(dirname, &f_stat);
      updateSkipDepth(&path, false);
    }
    else
      res = 0;
  }
  else if (!doit(match, &path, &src_stat, dirname, &f_stat)) {
      // TODO: message
  }
  else
    res = 0;

  return res;
}

#include "vunify-init.hc"

int main(int argc, char *argv[])
{
  struct Arguments	args = {
    .mode		=  mdVSERVER,
    .do_revert		=  false,
    .do_dry_run		=  false,
    .verbosity		=  0,
    .local_fs		=  false,
    .do_renew		=  true,
  };

  global_args = &args;
  while (1) {
    int		c = getopt_long(argc, argv, "Rnvcx",
				CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_MANUALLY		:  args.mode = mdMANUALLY; break;
      case 'R'			:  args.do_revert  = true; break;
      case 'n'			:  args.do_dry_run = true; break;
      case 'x'			:  args.local_fs   = true; break;
      //case 'C'			:  args.do_renew   = false; break;
      case 'v'			:  ++args.verbosity; break;
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
    case mdMANUALLY	:  initModeManually(&args, argc-optind, argv+optind); break;
    case mdVSERVER	:  initModeVserver (&args, argc-optind, argv+optind); break;
    default		:  assert(false); return EXIT_FAILURE;
  }
    
  global_info.state.d = "";
  global_info.state.l = 0;


  if (Global_getVerbosity()>=1) WRITE_MSG(1, "Starting to traverse directories...\n");
  Echdir(global_info.dst_list.root.d);
  visitDir("/", 0);

#ifndef NDEBUG
  {
    size_t		i;
    MatchList_destroy(&global_info.dst_list);
    for (i=0; i<global_info.src_lists.l; ++i)
      MatchList_destroy(global_info.src_lists.v+i);

    free(global_info.src_lists.v);
  }
#endif
}
