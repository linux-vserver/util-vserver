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

#include "pathinfo.h"
#include <string.h>
#include <assert.h>

#define CHECK(LHS,RHS, EXP)				\
  do {							\
    PathInfo	lhs  = { LHS, sizeof(LHS)-1 };		\
    PathInfo	rhs  = { RHS, sizeof(RHS)-1 };		\
    char	*buf = malloc(ENSC_PI_APPSZ(lhs,rhs));	\
    assert(ENSC_PI_APPSZ(lhs,rhs)>=sizeof(EXP));	\
    PathInfo_append(&lhs, &rhs, buf);			\
    assert(memcmp(lhs.d, EXP, sizeof(EXP))==0);		\
    assert(lhs.l == sizeof(EXP)-1);			\
    free(buf);						\
  } while (0)


void
PathInfo_test()
{
  CHECK("/var",  "/tmp", "/var/tmp");
  CHECK("/var",   "tmp", "/var/tmp");
  CHECK("/var/", "/tmp", "/var/tmp");
  CHECK("/var/",  "tmp", "/var/tmp");
  
  CHECK("/",  "tmp",   "/tmp");
  CHECK("/", "/tmp",   "/tmp");
  
  CHECK("", "/tmp",   "/tmp");
  
  CHECK("", "tmp",    "tmp");
  CHECK("", "",       "");
}
