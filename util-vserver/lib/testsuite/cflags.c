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

#include "lib_internal/coreassert.h"
#include "vserver.h"

#include <string.h>
#include <unistd.h>


#define TEST_T2F_C(X,Y,Z) assert(vc_text2cflag_compat(X,Y)==Z)
#define TEST_F2T_C(Y,X) {				\
    char const *x=vc_hicflag2text_compat(X);		\
    assert((x==0 && Y==0) || (x!=0 && Y!=0));		\
    if (x!=0 && Y!=0) assert(strcmp(x, Y)==0);		\
  }

#define TEST_LIST_C(STR,LEN,EXP,ERR_POS,ERR_LEN) {	\
    struct vc_err_listparser	err;			\
    char			buf[] = STR;		\
    uint32_t			res;			\
    res = vc_list2cflag_compat(buf, LEN, &err);		\
    assert(res==(EXP));					\
    assert(err.len==ERR_LEN);				\
    if (ERR_POS==-1) assert(err.ptr==0);		\
    else             assert(err.ptr==buf+(ERR_POS));	\
  }

//----

#define TEST_T2F(X,Y,Z) assert(vc_text2cflag(X,Y)==Z)
#define TEST_F2T(Y,X) {						\
    uint_least64_t x = (X);					\
    char const *rc=vc_locflag2text(&x);				\
    assert((rc==0 && Y==0) || (rc!=0 && Y!=0));			\
    if (rc!=0 && Y!=0) assert(strcmp(rc, Y)==0);		\
  }
#define TEST_LIST(STR,LEN,EXP_RES,EXP_FLAG,EXP_MASK,ERR_POS,ERR_LEN) {	\
    struct vc_err_listparser	err;					\
    char			buf[] = STR;				\
    volatile int		res;					\
    struct vc_ctx_flags		flags = {0,0};				\
    res = vc_list2cflag(buf, LEN, &err, &flags);			\
    assert(res==(EXP_RES));						\
    assert(flags.flagword==(uint_least64_t)(EXP_FLAG));			\
    assert(flags.mask    ==(uint_least64_t)(EXP_MASK));			\
    assert(err.len==ERR_LEN);						\
    if (ERR_POS==-1) assert(err.ptr==0);				\
    else             assert(err.ptr==buf+(ERR_POS));			\
  }

#define ALL64		(~(uint_least64_t)(0))

int main()
{
  TEST_T2F_C("lock",     0, S_CTX_INFO_LOCK);
  TEST_T2F_C("lockXXXX", 4, S_CTX_INFO_LOCK);
  TEST_T2F_C("locXXXXX", 3, 0);
  TEST_T2F_C("sched",    0, S_CTX_INFO_SCHED);
  TEST_T2F_C("nproc",    0, S_CTX_INFO_NPROC);
  TEST_T2F_C("private",  0, S_CTX_INFO_PRIVATE);
  TEST_T2F_C("fakeinit", 0, S_CTX_INFO_INIT);
  TEST_T2F_C("hideinfo", 0, S_CTX_INFO_HIDEINFO);
  TEST_T2F_C("ulimit",   0, S_CTX_INFO_ULIMIT);
  TEST_T2F_C("XXX",      0, 0);
  TEST_T2F_C("",         0, 0);

  TEST_F2T_C("lock",     S_CTX_INFO_LOCK);
  TEST_F2T_C("sched",    S_CTX_INFO_SCHED);
  TEST_F2T_C("nproc",    S_CTX_INFO_NPROC);
  TEST_F2T_C("private",  S_CTX_INFO_PRIVATE);
  TEST_F2T_C("fakeinit", S_CTX_INFO_INIT);
  TEST_F2T_C("hideinfo", S_CTX_INFO_HIDEINFO);
  TEST_F2T_C("ulimit",   S_CTX_INFO_ULIMIT);
  TEST_F2T_C(0,          0);
  TEST_F2T_C("ulimit",   64 | 128 | 23 );
  TEST_F2T_C("fakeinit", 23);

  TEST_LIST_C("lock",         0, S_CTX_INFO_LOCK,                  -1,0);
  TEST_LIST_C("lock,sched,",  0, S_CTX_INFO_LOCK|S_CTX_INFO_SCHED, -1,0);
  TEST_LIST_C("lock,XXX",     0, S_CTX_INFO_LOCK,                   5,3);
  TEST_LIST_C("",             0, 0,                                -1,0);
  TEST_LIST_C("X",            0, 0,                                 0,1);
  TEST_LIST_C("lock,sched,", 10, S_CTX_INFO_LOCK|S_CTX_INFO_SCHED, -1,0);

  //-------

  TEST_T2F("fakeinit", 0, VC_VXF_INFO_INIT);
  TEST_T2F("XXX",      0, 0);
  TEST_T2F("",         0, 0);
  
  TEST_F2T("fakeinit", VC_VXF_INFO_INIT);
  TEST_F2T(0,          0);

  TEST_LIST("fakeinit",     0,  0, VC_VXF_INFO_INIT, VC_VXF_INFO_INIT,-1,0);
  TEST_LIST("FaKeInIt",     0,  0, VC_VXF_INFO_INIT, VC_VXF_INFO_INIT,-1,0);
  TEST_LIST("~fakeinit",    0,  0, 0,               VC_VXF_INFO_INIT,-1,0);
  TEST_LIST("!fakeinit",    0,  0, 0,               VC_VXF_INFO_INIT,-1,0);
  TEST_LIST("fakeinit,XXX", 0, -1, VC_VXF_INFO_INIT, VC_VXF_INFO_INIT, 9,3);
  TEST_LIST("",             0,  0, 0,               0,              -1,0);
  TEST_LIST("0",            0,  0, 0,               0,              -1,0);
  TEST_LIST("00",           0,  0, 0,               0,              -1,0);
  TEST_LIST("X",            0, -1, 0,               0,               0,1);
  TEST_LIST("all",          0,  0, ALL64,           ALL64,          -1,0);
  TEST_LIST("ALL",          0,  0, ALL64,           ALL64,          -1,0);
  TEST_LIST("any",          0,  0, ALL64,           ALL64,          -1,0);
  TEST_LIST("ANY",          0,  0, ALL64,           ALL64,          -1,0);
  TEST_LIST("~all",         0,  0, 0,               ALL64,          -1,0);
  TEST_LIST("~ALL",         0,  0, 0,               ALL64,          -1,0);
  TEST_LIST("none",         0,  0, 0,               0,              -1,0);
  TEST_LIST("NONE",         0,  0, 0,               0,              -1,0);
  TEST_LIST("~none",        0,  0, 0,               0,              -1,0);
  TEST_LIST("~NONE",        0,  0, 0,               0,              -1,0);
  TEST_LIST("all,~fakeinit",0,  0, ~VC_VXF_INFO_INIT,ALL64,         -1,0);
  TEST_LIST("~all,fakeinit",0,  0, VC_VXF_INFO_INIT, ALL64,         -1,0);
  TEST_LIST("fakeinit,~all",0,  0, 0,               ALL64,          -1,0);
  TEST_LIST("none,~lock",   0,  0, 0,               VC_VXF_INFO_LOCK,-1,0);
  TEST_LIST("~none,lock",   0,  0, VC_VXF_INFO_LOCK,VC_VXF_INFO_LOCK,-1,0);
  TEST_LIST("lock,none",    0,  0, VC_VXF_INFO_LOCK,VC_VXF_INFO_LOCK,-1,0);
  TEST_LIST("lock,~none",   0,  0, VC_VXF_INFO_LOCK,VC_VXF_INFO_LOCK,-1,0);
  TEST_LIST("~",            0, -1, 0,               0,               1,0);
  TEST_LIST("~~",           0, -1, 0,               0,               2,0);
  TEST_LIST("!",            0, -1, 0,               0,               1,0);
  TEST_LIST("^",            0, -1, 0,               0,               1,0);
  TEST_LIST("fakeinit,~",   0, -1, VC_VXF_INFO_INIT, VC_VXF_INFO_INIT,10,0);
  TEST_LIST("1",            0,  0, 1,               1,              -1,0);
  TEST_LIST("1,23,42",      0,  0, 1|23|42,         1|23|42,        -1,0);
  TEST_LIST("~1",           0,  0, 0,               1,              -1,0);
  TEST_LIST("!1",           0,  0, 0,               1,              -1,0);
  TEST_LIST("~~1",          0,  0, 1,               1,              -1,0);
  TEST_LIST("~~~1",         0,  0, 0,               1,              -1,0);
  TEST_LIST("~!~1",         0,  0, 0,               1,              -1,0);
  TEST_LIST("42,fakeinit",  0,  0, VC_VXF_INFO_INIT|42, VC_VXF_INFO_INIT|42, -1,0);
  TEST_LIST("42x,1",        0, -1, 0,               0,               0,3);

  TEST_LIST("^4,~^2",       0,  0, 0x10,            0x14,           -1,0);
  TEST_LIST("^4,~~^2",      0,  0, 0x14,            0x14,           -1,0);
  TEST_LIST("^4,~~~^2",     0,  0, 0x10,            0x14,           -1,0);
  TEST_LIST("~^2,^4",       0,  0, 0x10,            0x14,           -1,0);
  TEST_LIST("1,^1,~^2,8",   0,  0, 0x0b,            0x0f,           -1,0);

  TEST_LIST("lock,nproc,private,fakeinit,hideinfo,ulimit,namespace,"
	    "sched_hard,sched_prio,sched_pause,"
	    "virt_mem,virt_uptime,virt_cpu,"
	    "hide_mount,hide_netif,state_setup,state_init",
	    0, 0,
	    VC_VXF_INFO_LOCK|VC_VXF_INFO_NPROC|VC_VXF_INFO_PRIVATE|VC_VXF_INFO_INIT|
	    VC_VXF_INFO_HIDEINFO|VC_VXF_INFO_ULIMIT|VC_VXF_INFO_NAMESPACE|
	    VC_VXF_SCHED_HARD|VC_VXF_SCHED_PRIO|VC_VXF_SCHED_PAUSE|
	    VC_VXF_VIRT_MEM|VC_VXF_VIRT_UPTIME|VC_VXF_VIRT_CPU|
	    VC_VXF_HIDE_MOUNT|VC_VXF_HIDE_NETIF|
	    VC_VXF_STATE_SETUP|VC_VXF_STATE_INIT,

	    VC_VXF_INFO_LOCK|VC_VXF_INFO_NPROC|VC_VXF_INFO_PRIVATE|VC_VXF_INFO_INIT|
	    VC_VXF_INFO_HIDEINFO|VC_VXF_INFO_ULIMIT|VC_VXF_INFO_NAMESPACE|
	    VC_VXF_SCHED_HARD|VC_VXF_SCHED_PRIO|VC_VXF_SCHED_PAUSE|
	    VC_VXF_VIRT_MEM|VC_VXF_VIRT_UPTIME|VC_VXF_VIRT_CPU|
	    VC_VXF_HIDE_MOUNT|VC_VXF_HIDE_NETIF|
	    VC_VXF_STATE_SETUP|VC_VXF_STATE_INIT,

	    -1,0);

  TEST_LIST("~lock,~nproc,~private,~fakeinit,~hideinfo,~ulimit,~namespace,"
	    "~sched_hard,~sched_prio,~sched_pause,"
	    "~virt_mem,~virt_uptime,~virt_cpu,"
	    "~hide_mount,~hide_netif,~state_setup,~state_init",
	    0, 0,
	    0,
	    VC_VXF_INFO_LOCK|VC_VXF_INFO_NPROC|VC_VXF_INFO_PRIVATE|VC_VXF_INFO_INIT|
	    VC_VXF_INFO_HIDEINFO|VC_VXF_INFO_ULIMIT|VC_VXF_INFO_NAMESPACE|
	    VC_VXF_SCHED_HARD|VC_VXF_SCHED_PRIO|VC_VXF_SCHED_PAUSE|
	    VC_VXF_VIRT_MEM|VC_VXF_VIRT_UPTIME|VC_VXF_VIRT_CPU|
	    VC_VXF_HIDE_MOUNT|VC_VXF_HIDE_NETIF|
	    VC_VXF_STATE_SETUP|VC_VXF_STATE_INIT,

	    -1,0);
  
	    
  return 0;
}
