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
#include <string.h>

MatchType
MatchList_compare(struct MatchList const *list, char const *path)
{
  struct MatchItem const *		ptr = list->data;
  struct MatchItem const * const	end_ptr = list->data + list->count;
  
  //write(1, path, strlen(path));
  //write(1, "\n", 1);
  for (; ptr<end_ptr; ++ptr) {
    if ((ptr->cmp==0 && strcmp(ptr->name, path)==0) ||
	(ptr->cmp!=0 && (ptr->cmp)(ptr->name, path)==0))
      return ptr->type;
  }

  return list->skip_depth > 0 ? stEXCLUDE : stINCLUDE;
}
