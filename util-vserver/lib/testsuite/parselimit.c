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

#include "lib_internal/coreassert.h"
#include "vserver.h"

#define TEST(BUF, RES, VAL)			\
  do {						\
    vc_limit_t		res = 0xdeadbeaf;	\
    assert(vc_parseLimit(BUF, &res)==(RES));	\
    assert(res==(VAL));				\
  } while (0)

#define TESTT(BUF,VAL)	 TEST(BUF, true,  VAL)
#define TESTF(BUF,VAL)	 TEST(BUF, false, VAL)

int main()
{
  TESTT("0", 0);
  TESTT("1", 1);
  TESTT("1k",     1000);
  TESTT("1K",     1024);
  TESTT("1m",     1000000);
  TESTT("1M",     1048576);
  TESTT("1234",   1234);
  TESTT("1234\n", 1234);
  TESTT("inf",    VC_LIM_INFINITY);

  TESTF("x", 0);
  TESTF("k", 0);
  TESTF("1kX", 1000);
  TESTF("",    0);

  return EXIT_SUCCESS;
}
