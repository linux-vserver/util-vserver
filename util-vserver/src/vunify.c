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
#include "wrappers-dirent.h"
#include "util.h"

#include <lib/vserver.h>

#include <getopt.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

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
	    "                       mode only and will be ignored for vserver unification\n"
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

#include "vunify-compare.ic"

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
    if ((!global_args->do_revert && !compareUnify  (*dst_fstat, src_fstat)) ||
	( global_args->do_revert && !compareDeUnify(*dst_fstat, src_fstat)))
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
	     compareDeUnify(cache_stat, src_stat)) {
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

inline static void
EsafeChdir(char const *path, struct stat const *exp_stat)
{
  FatalErrnoError(safeChdir(path, exp_stat)==-1, "safeChdir()");
}

#include "vunify-doit.ic"

static bool
doit(struct MatchList const *mlist,
     PathInfo const *src_path, struct stat const *src_stat,
     char const *dst_path,     struct stat const *dst_stat)
{
  PathInfo	path = mlist->root;
  char		path_buf[ENSC_PI_APPSZ(path, *src_path)];

  if (global_args->do_dry_run || global_args->verbosity>0) {
    if (global_args->do_revert) WRITE_MSG(1, "deunifying '");
    else                        WRITE_MSG(1, "unifying   '");

    write(1, src_path->d, src_path->l);
    WRITE_MSG(1, "'");

    if (global_args->verbosity>2) {
      WRITE_MSG(1, " (from ");
      if (global_args->verbosity==2 && mlist->id.d)
	write(1, mlist->id.d, mlist->id.l);
      else
	write(1, mlist->root.d, mlist->root.l);
      WRITE_MSG(1, ")");
    }
    WRITE_MSG(1, "\n");
  }
  
  PathInfo_append(&path, src_path, path_buf);
  return (global_args->do_dry_run ||
	  (!global_args->do_revert && doitUnify(  path.d, src_stat, dst_path, dst_stat)) ||
	  ( global_args->do_revert && doitDeUnify(path.d, src_stat, dst_path, dst_stat)));
}

static void
printListId(struct MatchList const *l)
{
  if (l->id.l>0) {
    WRITE_MSG(1, "'");
    write(1, l->id.d, l->id.l);
    WRITE_MSG(1, "'");
  }
  else if (l->root.l>0) {
    write(1, l->root.d, l->root.l);
  }
  else
    WRITE_MSG(1, "???");
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
      printListId(skip_reason.d.list);
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

static void
visitDirEntry(struct dirent const *ent)
{
  bool				is_dir;
  struct MatchList const *	match;
  struct stat			f_stat;
  char const *			dirname  = ent->d_name;
  PathInfo			path     = global_info.state;
  PathInfo			d_path = {
    .d = dirname,
    .l = strlen(ent->d_name)
  };
  char				path_buf[ENSC_PI_APPSZ(path, d_path)];
  bool				is_dotfile;
  struct stat			src_stat;

  PathInfo_append(&path, &d_path, path_buf);

  is_dotfile = (dirname[0]=='.' &&
		(dirname[1]=='\0' || (dirname[1]=='.' && dirname[2]=='\0')));
  memset(&f_stat, 0, sizeof f_stat);
  skip_reason.r = rsDOTFILE;

  if (is_dotfile ||
      (match=checkDirEntry(&path, &d_path, &is_dir, &src_stat, &f_stat))==0) {
    bool	is_link = is_dotfile ? false : S_ISLNK(f_stat.st_mode);
    
    if (global_args->verbosity>1 &&
	((!is_dotfile && !is_link) ||
	 (global_args->verbosity>4 && is_dotfile) ||
	 (global_args->verbosity>4 && is_link)) ) {
      WRITE_MSG(1, "  skipping '");
      write(1, path.d, path.l);
      WRITE_MSG(1, "'");
      if (global_args->verbosity>2) printSkipReason();
      WRITE_MSG(1, "\n");
    }
    return;
  }

  if (is_dir) {
    if (updateSkipDepth(&path, true)) {
      visitDir(dirname, &f_stat);
      updateSkipDepth(&path, false);
    }
  }
  else if (!doit(match, &path, &src_stat, dirname, &f_stat)) {
      // TODO: message
  }
}

static void
visitDir(char const *name, struct stat const *expected_stat)
{
  int		fd = Eopen(".", O_RDONLY, 0);
  PathInfo	old_state = global_info.state;
  PathInfo	rhs_path = {
    .d = name,
    .l = strlen(name)
  };
  char		new_path[ENSC_PI_APPSZ(global_info.state, rhs_path)];
  DIR *		dir;

  PathInfo_append(&global_info.state, &rhs_path, new_path);

  if (expected_stat!=0)
    EsafeChdir(name, expected_stat);
  
  dir = Eopendir(".");

  for (;;) {
    struct dirent		*ent = Ereaddir(dir);
    if (ent==0) break;

    visitDirEntry(ent);
  }

  Eclosedir(dir);

  Efchdir(fd);
  Eclose(fd);

  global_info.state = old_state;
}

#include "vunify-init.ic"

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


  if (global_args->verbosity>3) WRITE_MSG(1, "Starting to traverse directories...\n");
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
