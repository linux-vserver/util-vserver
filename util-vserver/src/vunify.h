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


#ifndef H_UTIL_VSERVER_SRC_VUNIFY_H
#define H_UTIL_VSERVER_SRC_VUNIFY_H

#include "lib_internal/matchlist.h"

struct dirent;
struct WalkdownInfo
{
    PathInfo				state;
    struct MatchList			dst_list;
    struct {
	struct MatchList *		v;
	size_t				l;
    }					src_lists;
};

struct SkipReason {
    enum { rsDOTFILE, rsEXCL_DST, rsEXCL_SRC,
	   rsFSTAT, rsNOEXISTS, rsSYMLINK, rsUNIFIED,
	   rsSPECIAL, rsDIFFERENT }	r;

    union {
	struct MatchList const *	list;
    }					d;
};

struct Arguments {
    enum {mdMANUALLY, mdVSERVER}	mode;
    bool				do_revert;
    bool				do_dry_run;
    unsigned int			verbosity;
    bool				local_fs;
    bool				do_renew;
};

struct stat;

static void	visitDirEntry(struct dirent const *) NONNULL((1));
static void	visitDir(char const *, struct stat const *) NONNULL((1));
static bool	checkFstat(struct MatchList const * const,
			   PathInfo const * const,
			   PathInfo const * const,
			   struct stat const ** const, struct stat * const,
			   struct stat *) NONNULL((1,2,3,4,5,6,7));

static struct MatchList const *
checkDirEntry(PathInfo const *,
	      PathInfo const *,
	      bool *, struct stat *, struct stat *) NONNULL((1,2,3,4,5));

static bool	updateSkipDepth(PathInfo const *, bool) NONNULL((1));
static void	EsafeChdir(char const *, struct stat const *)  NONNULL((1,2));
static bool	doit(struct MatchList const *,
		     PathInfo const *, struct stat const *,
		     char const *dst_path, struct stat const *) NONNULL((1,2,3));

#endif	//  H_UTIL_VSERVER_SRC_VUNIFY_H
