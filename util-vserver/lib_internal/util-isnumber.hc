// $Id$    --*- c -*--

// Copyright (C) 2006 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#define ENSC_DECL_UTIL_ISNUMBER(NAME,TYPE,FUNC)			\
  bool								\
  NAME(char const *str, TYPE *result, bool is_strict)		\
  {								\
    char *		errptr;					\
    TYPE		val;					\
    unsigned int	fac = 1;				\
								\
    errno = 0;							\
    val   = FUNC(str, &errptr, 0);				\
    if (errno==ERANGE)						\
      return false;						\
    if (errptr!=str && !is_strict) {				\
      switch (*errptr) {					\
	case 'M'	:  fac *= 1024; /* fallthrough */	\
	case 'K'	:  fac *= 1024; ++errptr; break;	\
	case 'm'	:  fac *= 1000; /* fallthrough */	\
	case 'k'	:  fac *= 1000; ++errptr; break;	\
	default	:  break;					\
      }								\
    }								\
    if (!checkConstraints(val,fac))				\
      return false;						\
								\
    if (*errptr!='\0' || errptr==str)				\
      return false;						\
    else {							\
      if (result) *result = val*fac;				\
      return true;						\
    }								\
  }
