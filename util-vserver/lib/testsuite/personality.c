// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include <string.h>
#include <unistd.h>
#include <linux/personality.h>


#define TEST_T2PF(X,Y,Z) assert(vc_text2personalityflag(X,Y)==Z)
#define TEST_PF2T(Y,X) {					\
    uint_least32_t x = (X);					\
    char const *rc=vc_lopersonality2text(&x);			\
    assert((rc==0 && Y==0) || (rc!=0 && Y!=0));			\
    if (rc!=0 && Y!=0) assert(strcmp(rc, Y ? Y : "")==0);	\
  }

#define TEST_LIST(STR,LEN,EXP_RES,EXP_PERS,ERR_POS,ERR_LEN) {		\
    struct vc_err_listparser	err;					\
    char			buf[] = STR;				\
    volatile int		res;					\
    uint_least32_t		personality = 0;			\
    res = vc_list2personalityflag(buf, LEN, &personality, &err);	\
    assert(res==(EXP_RES));						\
    assert(personality==(uint_least32_t)(EXP_PERS));			\
    assert(err.len==ERR_LEN);						\
    if (ERR_POS==-1) assert(err.ptr==0);				\
    else             assert(err.ptr==buf+(ERR_POS));			\
  }
    


#define TEST_T2PT(X,Y,Z) assert(vc_str2personalitytype(X,Y)==Z)



int main()
{
  TEST_T2PF("mmap_page_zero",      0, MMAP_PAGE_ZERO);
  TEST_T2PF("MMAP_PAGE_ZERO",      0, MMAP_PAGE_ZERO);
  TEST_T2PF("MmAp_PaGe_ZeRo",      0, MMAP_PAGE_ZERO);
  TEST_T2PF("mmap_page_zero",     14, MMAP_PAGE_ZERO);
  TEST_T2PF("MMAP_PAGE_ZERO",     14, MMAP_PAGE_ZERO);
  TEST_T2PF("MMAP_PAGE_ZEROXXXX", 14, MMAP_PAGE_ZERO);
  TEST_T2PF("MMAP_PAGE_ZEROXXXX", 13, 0);
  TEST_T2PF("MMAP_PAGE_ZERO",     13, 0);
  TEST_T2PF("MMAP_PAGE_ZERXXX",   13, 0);
  TEST_T2PF("XXX",                 0, 0);
  TEST_T2PF("",                    0, 0);
  
  TEST_T2PF("ADDR_LIMIT_32BIT",    0, ADDR_LIMIT_32BIT);
  TEST_T2PF("SHORT_INODE",         0, SHORT_INODE);
  TEST_T2PF("WHOLE_SECONDS",       0, WHOLE_SECONDS);
  TEST_T2PF("STICKY_TIMEOUTS",     0, STICKY_TIMEOUTS);
  TEST_T2PF("ADDR_LIMIT_3GB",      0, ADDR_LIMIT_3GB);


    // the  _loc* tests
  TEST_PF2T("MMAP_PAGE_ZERO",      MMAP_PAGE_ZERO);
  TEST_PF2T("ADDR_LIMIT_32BIT",    ADDR_LIMIT_32BIT);
  TEST_PF2T("SHORT_INODE",         SHORT_INODE);
  TEST_PF2T("WHOLE_SECONDS",       WHOLE_SECONDS);
  TEST_PF2T("STICKY_TIMEOUTS",     STICKY_TIMEOUTS);
  TEST_PF2T("ADDR_LIMIT_3GB",      ADDR_LIMIT_3GB);



  TEST_LIST("mmap_page_zero",      14,  0, MMAP_PAGE_ZERO, -1, 0);
  TEST_LIST("mmap_page_zero,XXX",  14,  0, MMAP_PAGE_ZERO, -1, 0);
  TEST_LIST("mmap_page_zero",       0,  0, MMAP_PAGE_ZERO, -1, 0);
  TEST_LIST("MmAp_pAgE_ZeRo",       0,  0, MMAP_PAGE_ZERO, -1, 0);
  TEST_LIST("mmap_page_zero,XXX",   0, -1, MMAP_PAGE_ZERO, 15, 3);
  TEST_LIST("~mmap_page_zero",      0, -1,              0,  0,15);
  TEST_LIST("!mmap_page_zero",      0, -1,              0,  0,15);
  TEST_LIST("",                     0,  0,              0, -1, 0);
  TEST_LIST("0",                    0,  0,              0, -1, 0);
  TEST_LIST("00",                   0,  0,              0, -1, 0);
  TEST_LIST("1",                    0,  0,              1, -1, 0);
  TEST_LIST("1,23,42",              0,  0,        1|23|42, -1, 0);
  TEST_LIST("^1",                   0,  0,              2, -1, 0);
  TEST_LIST("^4,^2",                0,  0,           0x14, -1, 0);
  TEST_LIST("^2,^3",                0,  0,           0x0c, -1, 0);
  TEST_LIST("^2,~^3",               0, -1,           0x04,  3, 3);
  TEST_LIST("~0",                   0, -1,              0,  0, 2);
  TEST_LIST("^",                    0, -1,              0,  1, 0);
  TEST_LIST("~",                    0, -1,              0,  0, 1);
  TEST_LIST("!",                    0, -1,              0,  0, 1);
  TEST_LIST("X",                    0, -1,              0,  0, 1);
  TEST_LIST("all",                  0, -1,              0,  0, 3);
  TEST_LIST("ALL",                  0, -1,              0,  0, 3);
  TEST_LIST("~all",                 0, -1,              0,  0, 4);
  TEST_LIST("~ALL",                 0, -1,              0,  0, 4);
  TEST_LIST("any",                  0, -1,              0,  0, 3);
  TEST_LIST("ANY",                  0, -1,              0,  0, 3);
  TEST_LIST("~any",                 0, -1,              0,  0, 4);
  TEST_LIST("~ANY",                 0, -1,              0,  0, 4);
  TEST_LIST("none",                 0, -1,              0,  0, 4);
  TEST_LIST("NONE",                 0, -1,              0,  0, 4);
  TEST_LIST("~none",                0, -1,              0,  0, 5);
  TEST_LIST("~NONE",                0, -1,              0,  0, 5);
  TEST_LIST("mmap_page_zero,all",   0, -1, MMAP_PAGE_ZERO, 15, 3);
  TEST_LIST("mmap_page_zero,any",   0, -1, MMAP_PAGE_ZERO, 15, 3);

  TEST_LIST("mmap_page_zero,addr_limit_32bit,short_inode,whole_seconds,"
	    "sticky_timeouts,addr_limit_3gb",
	    0, 0,
	    MMAP_PAGE_ZERO|ADDR_LIMIT_32BIT|SHORT_INODE|WHOLE_SECONDS|
	    STICKY_TIMEOUTS|ADDR_LIMIT_3GB,
	    -1, 0);

  TEST_T2PT("linux",     0, PER_LINUX);
  TEST_T2PT("LINUX",     0, PER_LINUX);
  TEST_T2PT("LiNuX",     0, PER_LINUX);
  TEST_T2PT("LiNuX",     5, PER_LINUX);
  TEST_T2PT("LiNuX",     4, VC_BAD_PERSONALITY);
  TEST_T2PT("LiNuXAAA",  5, PER_LINUX);
  TEST_T2PT("LiNuXAAA",  4, VC_BAD_PERSONALITY);
  TEST_T2PT("LiNuAAA",   4, VC_BAD_PERSONALITY);
  TEST_T2PT("XXX",       0, VC_BAD_PERSONALITY);
  TEST_T2PT("",          0, VC_BAD_PERSONALITY);

  
  return 0;
}
