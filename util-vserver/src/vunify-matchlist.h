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


#ifndef H_UTIL_VSERVER_SRC_VUNIFY_MATCHLIST_H
#define H_UTIL_VSERVER_SRC_VUNIFY_MATCHLIST_H

#include "util.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

typedef int	(*MatchItemCompareFunc)(char const *, char const *);

struct MatchItem
{
    enum { stINCLUDE, stEXCLUDE }	type;
    char const *			name;
    MatchItemCompareFunc		cmp;
};

typedef struct
{
    char const *	d;
    size_t		l;
} String;

typedef String		PathInfo;

struct MatchList
{
    size_t		skip_depth;
    PathInfo		root;
    String		id;
    struct MatchItem	*data;
    size_t		count;

    void const		**buf;
    size_t		buf_count;
};


void		MatchList_init(struct MatchList *, char const *root, size_t count) NONNULL((1,2));
void		MatchList_destroy(struct MatchList *) NONNULL((1));
void		MatchList_appendFiles(struct MatchList *, size_t idx,
				      char **files, size_t count,
				      bool auto_type);

bool		MatchList_compare(struct MatchList const *, char const *path) NONNULL((1,2));
struct MatchItem
const *		MatchList_find(struct MatchList const *,    char const *path) NONNULL((1,2));

void		String_init(String *);
void		String_destroy(String *);

void		PathInfo_append(PathInfo * restrict,
				PathInfo const * restrict,
				char *buf) NONNULL((1,2,3));

#define ENSC_PI_APPSZ(P1,P2)	((P1).l + sizeof("/") + (P2).l)

#endif	//  H_UTIL_VSERVER_SRC_VUNIFY_MATCHLIST_H
