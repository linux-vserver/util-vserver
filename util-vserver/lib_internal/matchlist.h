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


#ifndef H_UTIL_VSERVER_LIB_INTERNAL_MATCHLIST_H
#define H_UTIL_VSERVER_LIB_INTERNAL_MATCHLIST_H

#include "pathinfo.h"
#include "string.h"

#include <stdlib.h>
#include <stdbool.h>

typedef int	(*MatchItemCompareFunc)(char const *, char const *);
typedef enum { stINCLUDE,stEXCLUDE,stSKIP }	MatchType;

struct MatchItem
{
    MatchType			type;
    char const *		name;
    MatchItemCompareFunc	cmp;
};

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

void		MatchList_init(struct MatchList *, char const *root,
			       size_t count) NONNULL((1,2));
bool		MatchList_initByVserver(struct MatchList *,
					char const *vserver,
					char const **res_appdir) NONNULL((1,2));
void		MatchList_initManually(struct MatchList *list,
				       char const *vserver,
				       char const *vdir,
				       char const *exclude_file) NONNULL((1,3,4));
void		MatchList_initRefserverList(struct MatchList **, size_t *cnt,
					    char const *dir) NONNULL((1,2,3));
void		MatchList_destroy(struct MatchList *) NONNULL((1));
void		MatchList_appendFiles(struct MatchList *, size_t idx,
				      char **files, size_t count,
				      bool auto_type) NONNULL((1,3));

MatchType	MatchList_compare(struct MatchList const *,
				  char const *path) NONNULL((1,2));
struct MatchItem
const *		MatchList_find(struct MatchList const *,
			       char const *path) NONNULL((1,2));

void		MatchList_printId(struct MatchList const *, int fd) NONNULL((1));

#endif	//  H_UTIL_VSERVER_LIB_INTERNAL_MATCHLIST_H
