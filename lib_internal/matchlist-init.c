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
#include <stdlib.h>

#define ENSC_WRAPPERS_STDLIB	1
#include <wrappers.h>
void
MatchList_init(struct MatchList *list, char const *root, size_t count)
{
  list->skip_depth = 0;
  list->root.d     = root;
  list->root.l     = strlen(root);
  list->data       = Emalloc(sizeof(struct MatchItem) * count);
  list->count      = count;
  list->buf        = 0;
  list->buf_count  = 0;

  String_init(&list->id);
}
