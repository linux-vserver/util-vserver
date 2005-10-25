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

#include "lib/fmt.h"
#include <string.h>

#define TEST(VAL, EXP) {			\
    char	buf[512];			\
    size_t	l;				\
    memset(buf+1, '\23', sizeof(buf)-2);	\
    buf[0] = buf[sizeof(EXP)] = '\42';		\
    l = FUNC(buf+1, VAL);			\
    assert(l==sizeof(EXP)-1);			\
    assert(memcmp(buf+1, EXP, l)==0);		\
    assert(buf[0]  =='\42');			\
    assert(buf[l+1]=='\42');			\
    assert(buf[l+2]=='\23');			\
  }

int main()
{
  #define FUNC	utilvserver_fmt_xuint64
  TEST(0,  "0");
  TEST(1,   "1");
  TEST(15,  "f");
  TEST(16,  "10");
  TEST(100, "64");
  TEST(1000, "3e8");
  TEST(65535, "ffff");
  TEST(65536, "10000");
  TEST(68719476736ul, "1000000000");

  return 0;
}
