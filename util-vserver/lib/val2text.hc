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

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define VAL2TEXT(TYPE,SHORT)						\
  ssize_t								\
  utilvserver_value2text_##SHORT(char const *str, size_t len,		\
				 struct Mapping_ ##SHORT const *map,	\
				 size_t map_len)			\
  {									\
    size_t	i;							\
    if (len==0) len=strlen(str);					\
									\
    for (i=0; i<map_len; ++i)						\
      if (len==map[i].len &&						\
	  strncasecmp(map[i].id, str, len)==0)				\
	return i;							\
									\
    return -1;								\
  }

#define TEXT2VAL(TYPE,SHORT)						\
  static ssize_t							\
  searchValue(TYPE val,							\
	      struct Mapping_##SHORT const *map,  size_t map_len)	\
  {									\
    size_t		i;						\
    for (i=0; i<map_len; ++i)						\
      if (val == map[i].val) return i;					\
    return -1;								\
  }									\
									\
  ssize_t								\
  utilvserver_text2value_##SHORT(TYPE *val,				\
				 struct Mapping_##SHORT const *map,	\
				 size_t map_len)			\
  {									\
    ssize_t	idx;							\
    TYPE	del_val;						\
    if (*val==0)							\
      return -1;							\
									\
    del_val = *val;							\
    idx     = searchValue(del_val, map, map_len);			\
									\
    if (idx==-1) {							\
      size_t	i;							\
      for (i=0; i<sizeof(*val)*8 && (*val&(1<<i))==0; ++i) {}		\
      del_val = (1<<i);							\
      idx     = searchValue(del_val, map, map_len);			\
    }									\
    *val &= ~del_val;							\
    return idx;								\
  }
