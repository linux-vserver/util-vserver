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


#ifndef H_VSERVER_DJINNI_SRC_FMT_H
#define H_VSERVER_DJINNI_SRC_FMT_H

#include <stdlib.h>
#include <stdint.h>

#ifndef FMT_PREFIX
#  define FMT_PREFIX	fmt_
#endif

#define FMT_P__(X,Y)	X ## Y
#define FMT_P_(X,Y)	FMT_P__(X,Y)
#define FMT_P(X)	FMT_P_(FMT_PREFIX, X)

#define STRINGIFY_(X)	#X
#define STRINGIFY(X)	STRINGIFY_(X)
#define WEAKFUNC(X)	__attribute__((__weak__,__alias__(STRINGIFY(FMT_P(X)))))


#ifdef __cplusplus
extern "C" {
#endif

size_t	FMT_P(uint64_base)(char *ptr, uint_least64_t val, char base);
size_t	FMT_P( int64_base)(char *ptr,  int_least64_t val, char base);

size_t	FMT_P(uint32_base)(char *ptr, uint_least32_t val, char base);
size_t	FMT_P( int32_base)(char *ptr,  int_least32_t val, char base);

#if __WORDSIZE == 64
size_t	FMT_P(ulong_base)(char *ptr, unsigned long val, char base) WEAKFUNC(uint64_base);
size_t	FMT_P( long_base)(char *ptr,          long val, char base) WEAKFUNC( int64_base);
#else
size_t	FMT_P(ulong_base)(char *ptr, unsigned long val, char base) WEAKFUNC(uint32_base);
size_t	FMT_P( long_base)(char *ptr,          long val, char base) WEAKFUNC( int32_base);
#endif

size_t	FMT_P(uint_base)(char *ptr, unsigned int val, char base) WEAKFUNC(uint32_base);
size_t	FMT_P( int_base)(char *ptr,          int val, char base) WEAKFUNC( int32_base);


inline static size_t
FMT_P(ulong)(char *ptr, unsigned long val)
{
  return FMT_P(ulong_base)(ptr, val, 10);
}

inline static size_t
FMT_P(long)(char *ptr, long val)
{
  return FMT_P(long_base)(ptr, val, 10);
}


inline static size_t
FMT_P(uint)(char *ptr, unsigned int val)
{
  return FMT_P(uint_base)(ptr, val, 10);
}

inline static size_t
FMT_P(int)(char *ptr, int val)
{
  return FMT_P(int_base)(ptr, val, 10);
}


#ifdef __cplusplus
}
#endif

#undef WEAKFUNC
#undef STRINGIFY
#undef STRINGIFY_
#undef FMT_P
#undef FMT_P_
#undef FMT_P__

#endif	//  H_VSERVER_DJINNI_SRC_FMT_H
