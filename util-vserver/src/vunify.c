// $Id$    --*- c -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "vunify-matchlist.h"
#include "wrappers-dirent.h"
#include "vunify-operations.h"
#include "util.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

int	wrapper_exit_code = 1;

struct WalkdownInfo
{
    PathInfo			state;
    struct MatchList		dst_list;
    struct {
	struct MatchList *	v;
	size_t			l;
    }				src_lists;
};


static struct WalkdownInfo	global_info;
static struct Operations	operations;

static void	visitDirEntry(struct dirent const *) NONNULL((1));
static void	visitDir(char const *, struct stat const *) NONNULL((1));
static bool	checkFstat(struct MatchList const * const,
			   PathInfo const * const,
			   PathInfo const * const,
			   struct stat const ** const,
			   struct stat * const) NONNULL((1,2,3,4,5));

static struct MatchList const *
checkDirEntry(PathInfo const *,
	      PathInfo const *,
	      bool *, struct stat *) NONNULL((1,2,3,4));

static bool	updateSkipDepth(PathInfo const *, bool) NONNULL((1));
static void	EsafeChdir(char const *, struct stat const *)  NONNULL((1,2));
static bool	doit(struct MatchList const *, PathInfo const *,
		     char const *dst_path) NONNULL((1,2,3));



// Returns 'false' iff one of the files is not existing, or of the files are different/not unifyable
static bool
checkFstat(struct MatchList const * const mlist,
	   PathInfo const * const  basename,
	   PathInfo const * const  path,
	   struct stat const ** const result, struct stat * const result_buf)
{
  assert(basename->d[0] != '/');

  if (*result==0) {
    // local file does not exist... strange
    // TODO: message
    if (lstat(basename->d, result_buf)==-1) return false;
    *result = result_buf;
  }

  assert(*result!=0);
  
  {
    struct stat		src_fstat;
    PathInfo		src_path = mlist->root;
    char		src_path_buf[ENSC_PI_APPSZ(src_path, *path)];

    PathInfo_append(&src_path, path, src_path_buf);

    // source file does not exist
    if (lstat(src_path.d, &src_fstat)==-1) return false;

    // both files are different, so return false
    if (!(operations.compare)(*result, &src_fstat)) return false;
  }

  // these are the same files
  return true;
}

static struct MatchList const *
checkDirEntry(PathInfo const *path,
	      PathInfo const *d_path, bool *is_dir, struct stat *f_stat)
{
  struct WalkdownInfo const * const	info     = &global_info;
  struct MatchList const *		mlist;
  struct stat const *			cache_stat = 0;

  // Check if it is in the exclude/include list of the destination vserver and
  // abort when it is not matching an allowed entry
  if (!MatchList_compare(&info->dst_list, path->d)) return 0;

  // Now, go through the reference vservers and do the lightweigt list-check
  // first and compare then the fstat's.
  for (mlist=info->src_lists.v; mlist<info->src_lists.v+info->src_lists.l; ++mlist) {
    if (MatchList_compare(mlist, path->d) &&
	checkFstat(mlist, d_path, path, &cache_stat, f_stat)) {

      // Failed the check or is it a symlink which can not be handled
      if (cache_stat==0 || S_ISLNK(f_stat->st_mode)) return 0;
      
      *is_dir = S_ISDIR(f_stat->st_mode);
      return mlist;
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

static bool
doit(struct MatchList const *mlist, PathInfo const *src_path,
     char const *dst_path)
{
  PathInfo	path = mlist->root;
  char		path_buf[ENSC_PI_APPSZ(path, *src_path)];

  PathInfo_append(&path, src_path, path_buf);
  return (operations.doit)(path.d, dst_path);
}


static void
visitDirEntry(struct dirent const *ent)
{
  bool				is_dir;
  struct MatchList const *	match;
  struct stat			f_stat;
  char const *			dirname  = ent->d_name;
  size_t			path_len = strlen(ent->d_name);
  PathInfo			path     = global_info.state;
  PathInfo			d_path = {
    .d = dirname,
    .l = path_len
  };
  char				path_buf[ENSC_PI_APPSZ(path, d_path)];

  PathInfo_append(&path, &d_path, path_buf);

  if ((dirname[0]=='.' &&
       (dirname[1]=='\0' || (dirname[1]=='.' && dirname[2]=='\0'))) ||
      (match=checkDirEntry(&path, &d_path, &is_dir, &f_stat))==0) {
    WRITE_MSG(1, "skipping '");
    WRITE_STR(1, dirname);
    WRITE_MSG(1, "'\n");
    return;
  }

  if (is_dir) {
    if (updateSkipDepth(&path, true)) {
      visitDir(dirname, &f_stat);
      updateSkipDepth(&path, false);
    }
  }
  else if (!doit(match, &path, dirname)) {
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


int main(int argc, char *argv[])
{
  global_info.state.d = "";
  global_info.state.l = 0;

  Operations_init(&operations, opUNIFY, false);
  MatchList_init(&global_info.dst_list, argv[1], 0);

  global_info.src_lists.v = malloc(sizeof(struct MatchList));
  global_info.src_lists.l = 1;
  MatchList_init(global_info.src_lists.v+0, argv[2], 0);

  Echdir(global_info.dst_list.root.d);
  visitDir("/", 0);
}
