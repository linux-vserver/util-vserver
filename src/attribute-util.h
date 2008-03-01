// $Id$    --*- c -*--

// Copyright (C) 2007-2008 Daniel Hokka Zakrisson
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

#ifndef _ATTRIBUTE_UTIL_H
#define _ATTRIBUTE_UTIL_H

static inline int
ffsull(unsigned long long word)
{
  int bit;
  for (bit = 0; bit < 64; bit++) {
    if (word & (1ULL << bit))
      break;
  }
  if (bit == 64)
    bit = 0;
  return bit;
}

#define print_bitfield(fd, type, name, var)	\
  do {						\
  int first;					\
  WRITE_MSG(fd, name ":\n");			\
  first = 1;					\
  while (1) {					\
    char const *i;				\
    i = vc_lo ## type ## 2text(var);		\
    if (!i)					\
      break;					\
    if (!first)					\
      WRITE_MSG(fd, ",");			\
    else					\
      first = 0;				\
    WRITE_STR(fd, i);				\
  }						\
  while (*(var)) {				\
    int bit = ffsull(*(var));			\
    if (!bit)					\
      break;					\
    if (!first)					\
      WRITE_MSG(fd, ",");			\
    else					\
      first = 0;				\
    WRITE_MSG(fd, "^");				\
    WRITE_INT(fd, bit);				\
    *(var) &= ~(1ULL << bit);			\
  }						\
  WRITE_MSG(fd, "\n");				\
  } while (0)

#endif
