// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <ensc@delenn.intern.sigma-chemnitz.de>
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

#include "vserver.h"
#include <assert.h>
#include <string.h>

#define TEST_T2F(X,Y,Z) assert(vc_text2flag(X,Y)==Z)
#define TEST_F2T(Y,X) {				\
    char const *x=vc_hiflag2text(X);		\
    assert((x==0 && Y==0) || (x!=0 && Y!=0));	\
    if (x!=0) assert(strcmp(x, Y)==0);		\
  }

#define TEST_LIST(STR,LEN,EXP,ERR_POS,ERR_LEN) {		\
    char const	*err_ptr;					\
    size_t	err_len;					\
    char	buf[] = STR;					\
    uint32_t	res;						\
    res = vc_textlist2flag(buf, LEN, &err_ptr, &err_len);	\
    assert(res==(EXP));						\
    assert(err_len==ERR_LEN);					\
    if (ERR_POS==-1) assert(err_ptr==0);			\
    else             assert(err_ptr==buf+(ERR_POS));		\
  }


int main()
{
  TEST_T2F("lock",     0, S_CTX_INFO_LOCK);
  TEST_T2F("lockXXXX", 4, S_CTX_INFO_LOCK);
  TEST_T2F("locXXXXX", 3, 0);
  TEST_T2F("sched",    0, S_CTX_INFO_SCHED);
  TEST_T2F("nproc",    0, S_CTX_INFO_NPROC);
  TEST_T2F("private",  0, S_CTX_INFO_PRIVATE);
  TEST_T2F("init",     0, S_CTX_INFO_INIT);
  TEST_T2F("hideinfo", 0, S_CTX_INFO_HIDEINFO);
  TEST_T2F("ulimit",   0, S_CTX_INFO_ULIMIT);
  TEST_T2F("XXX",      0, 0);
  TEST_T2F("",         0, 0);

  TEST_F2T("lock",     S_CTX_INFO_LOCK);
  TEST_F2T("sched",    S_CTX_INFO_SCHED);
  TEST_F2T("nproc",    S_CTX_INFO_NPROC);
  TEST_F2T("private",  S_CTX_INFO_PRIVATE);
  TEST_F2T("init",     S_CTX_INFO_INIT);
  TEST_F2T("hideinfo", S_CTX_INFO_HIDEINFO);
  TEST_F2T("ulimit",   S_CTX_INFO_ULIMIT);
  TEST_F2T(0,          0);
  TEST_F2T("ulimit",   64 | 128 | 23 );
  TEST_F2T("init",     23);

  TEST_LIST("lock",         0, S_CTX_INFO_LOCK,                  -1,0);
  TEST_LIST("lock,sched,",  0, S_CTX_INFO_LOCK|S_CTX_INFO_SCHED, -1,0);
  TEST_LIST("lock,XXX",     0, S_CTX_INFO_LOCK,                   5,3);
  TEST_LIST("",             0, 0,                                -1,0);
  TEST_LIST("X",            0, 0,                                 0,1);
  TEST_LIST("lock,sched,", 10, S_CTX_INFO_LOCK|S_CTX_INFO_SCHED, -1,0);

  return 0;
}

