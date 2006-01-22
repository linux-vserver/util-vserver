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

#define ENSC_TESTSUITE

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "lib_internal/util.h"
#include "lib_internal/coreassert.h"

#define TEST_TMPL(FUNC, TYPE, STR, VAL, STRICT)			\
  do {								\
    TYPE	exp = (TYPE)(VAL)!=(TYPE)BAD ? (VAL) : 0;	\
    bool	val = (TYPE)(VAL)==(TYPE)BAD ? false : true;	\
    TYPE	tmp;						\
    bool	rc = FUNC((STR), &tmp, (STRICT));		\
    char const * const UNUSED STR_FUNC    = #FUNC;		\
    char const * const UNUSED STR_VAL     = #VAL;		\
    char const * const UNUSED STR_STRICT  = #STRICT;		\
    assert(rc == val);						\
    rc = FUNC((STR), 0, (STRICT));				\
    assert(val == rc);						\
    if (val) assert(tmp == exp);				\
  } while (0)

#define TESTS(STR, VAL, STRICT)			\
  TEST_TMPL(isNumber, signed long, STR, VAL, STRICT)

#define TESTU(STR, VAL, STRICT)					\
  TEST_TMPL(isNumberUnsigned, unsigned long, STR, VAL, STRICT)

#define TEST(STR, VALS0, VALS1, VALU0, VALU1)	\
  TESTS(STR, VALS0, true);			\
  TESTS(STR, VALS1, false);			\
  TESTU(STR, VALU0, true);			\
  TESTU(STR, VALU1, false);


#define BAD	0xdeadbeaf

int main()
{
  TEST( "0",        0,        0,    0,       0);
  TEST( "1",        1,        1,    1,       1);
  TEST("-1",       -1,       -1,  BAD,     BAD);
  TEST( "1k",     BAD,     1000,  BAD,    1000);
//TEST("-1k",     BAD,    -1000,  BAD,     BAD);
  TEST( "1K",     BAD,     1024,  BAD,    1024);
//TEST("-1K",     BAD,    -1024,  BAD,     BAD);
  TEST( "1m",     BAD,  1000000,  BAD, 1000000);
//TEST("-1m",     BAD, -1000000,  BAD,     BAD);
  TEST( "1M",     BAD,  1048576,  BAD, 1048576);
//TEST("-1M",     BAD, -1048576,  BAD,     BAD);
  
  TEST( "010",      8,        8,    8,       8);
  TEST( "010k",   BAD,     8000,  BAD,    8000);
  TEST("-010",     -8,       -8,  BAD,     BAD);
//TEST("-010k",   BAD,     8000,  BAD,     BAD);

  TEST( "0x10",    16,       16,   16,      16);
  TEST( "0x10k",  BAD,    16000,  BAD,   16000);
  TEST("-0x10",   -16,      -16,  BAD,     BAD);
//TEST("-0x10k",  BAD,   -16000,  BAD,     BAD);
}
