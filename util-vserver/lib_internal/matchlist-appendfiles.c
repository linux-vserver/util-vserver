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

#include "matchlist.h"
#include <fnmatch.h>
#include <assert.h>

static int
fnmatchWrap(char const *a, char const *b)
{
  return fnmatch(a, b, 0);
}

static MatchItemCompareFunc
determineCompareFunc(char const UNUSED *fname)
{
  return fnmatchWrap;
}

void
MatchList_appendFiles(struct MatchList *list, size_t idx,
		      char **files, size_t count,
		      bool auto_type)
{
  struct MatchItem	*ptr = list->data + idx;
  size_t		i;
  
  assert(idx+count <= list->count);

  if (auto_type) {
    for (i=0; i<count; ++i) {
      char	*file = files[i];
      switch (file[0]) {
	case '+'	:  ptr->type = stINCLUDE; ++file; break;
	case '-'	:  ++file; /*@fallthrough@*/
	default		:  ptr->type = stEXCLUDE; break;
      }
      ptr->cmp  = determineCompareFunc(file);
      ptr->name = file;
      ++ptr;
    }
  }
  else {
    for (i=0; i<count; ++i) {
      ptr->type = stEXCLUDE;
      ptr->name = files[i];
      ptr->cmp  = 0;
      ++ptr;
    }
  }
}
