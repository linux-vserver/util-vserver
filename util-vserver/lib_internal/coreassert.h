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


#ifndef H_UTILVSERVER_LIB_INTERNAL_COREASSERT_H
#define H_UTILVSERVER_LIB_INTERNAL_COREASSERT_H

#ifndef ENSC_TESTSUITE
#  error Do not use <coreassert.h> outside of testenvironemnts!
#endif

#include <assert.h>
#include <unistd.h>

#undef assert
#define ASSERT_STRX(X)	#X
#define ASSERT_STR(X)	ASSERT_STRX(X)
#define ASSERT_WRITE(X)	write(2, (X), sizeof(X)-1)
#define assert(X)					\
  (!(X)) ?						\
  ASSERT_WRITE(__FILE__ ":" ASSERT_STR(__LINE__)	\
	       " Assertion: '" #X "' failed\n"),	\
  *(char *)(0)=0 : 0


#endif	//  H_UTILVSERVER_LIB_INTERNAL_COREASSERT_H
