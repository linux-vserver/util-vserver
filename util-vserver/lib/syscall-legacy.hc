// $Id$ --*- c -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on syscall.cc by Jacques Gelinas
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/*
	This tells the system call number for new_s_context and set_ipv4root
	using /proc/self/status. This helps until the vserver project is
	included officially in the kernel (and has its own syscall).

	We rely on /proc/self/status to find the syscall number.

	If it is not there, we rely on adm/unistd.h.

	If this file does not have those system calls (not a patched kernel source)
	we rely on static values in this file.
*/
#include "safechroot-internal.hc"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <asm/unistd.h>
#include <stdbool.h>

#include "syscall-wrap.h"

// Here is the trick. We keep a copy of the define, then undef it
// and then later, we try to locate the value reading /proc/self/status
// If this fails, we have the old preserved copy.
static int def_NR_set_ipv4root = 274;
#undef __NR_set_ipv4root

static int __NR_set_ipv4root_rev0;
static int __NR_set_ipv4root_rev1;
static int __NR_set_ipv4root_rev2;
static int __NR_set_ipv4root_rev3;
static int rev_ipv4root=0;

#ifdef ENSC_SYSCALL_TRADITIONAL
#  if defined __dietlibc__
extern long int syscall (long int __sysno, ...);
#  endif

inline static int
set_ipv4root_rev0(unsigned long ip)
{
  return syscall(__NR_set_ipv4root_rev0, ip);
}

inline static int
set_ipv4root_rev1(unsigned long ip, unsigned long bcast)
{
  return syscall(__NR_set_ipv4root_rev1, ip, bcast);
}

inline static int
set_ipv4root_rev2(unsigned long *ip, int nb, unsigned long bcast)
{
  return syscall(__NR_set_ipv4root_rev2, ip, nb, bcast);
}

inline static int
set_ipv4root_rev3(unsigned long *ip, int nb, unsigned long bcast, unsigned long * mask)
{
  return syscall(__NR_set_ipv4root_rev3, ip, nb, bcast, mask);
}

#else  // ENSC_SYSCALL_TRADITIONAL
inline static _syscall1(int, set_ipv4root_rev0, unsigned long, ip)
inline static _syscall2(int, set_ipv4root_rev1, unsigned long, ip, unsigned long, bcast)
inline static _syscall3(int, set_ipv4root_rev2, unsigned long *, ip, int, nb, unsigned long, bcast)
inline static _syscall4(int, set_ipv4root_rev3, unsigned long *, ip, int, nb, unsigned long, bcast, unsigned long *, mask)
#endif // ENSC_SYSCALL_TRADITIONAL

static int def_NR_new_s_context = 273;
#undef __NR_new_s_context
static int __NR_new_s_context_rev0;
static int rev_s_context=0;


#ifdef ENSC_SYSCALL_TRADITIONAL
inline static xid_t
new_s_context_rev0(int newctx, int remove_cap, int flags)
{
  return syscall(__NR_new_s_context_rev0, newctx, remove_cap, flags);
}
#else  // ENSC_SYSCALL_TRADITIONAL
inline static _syscall3(int, new_s_context_rev0, int, newctx, int, remove_cap, int, flags)
#endif // ENSC_SYSCALL_TRADITIONAL


static bool	is_init = false;

#include "utils-legacy.h"

#ifndef WRITE_MSG
#  define WRITE_MSG(FD,X)         (void)(write(FD,X,sizeof(X)-1))
#endif


static bool
getNumRevPair(char const *str, int *num, int *rev)
{
  char const *	blank_pos = strchr(str, ' ');
  char const *	eol_pos   = strchr(str, '\n');
  
  *num = atoi(str);
  if (*num==0) return false;
  
  if (blank_pos!=0 && eol_pos!=0 && blank_pos<eol_pos &&
      strncmp(blank_pos+1, "rev", 3)==0)
    *rev = atoi(blank_pos+4);

  return true;
}

#define SET_TAG_POS(TAG)			\
  pos = strstr(buf, (TAG));			\
  if (pos) pos+=sizeof(TAG)-1

static bool init_internal()
{
  size_t			bufsize = utilvserver_getProcEntryBufsize();
  char				buf[bufsize];
  char const *			pos = 0;
  pid_t				pid = getpid();
  int				num;

  errno = 0;

  pos=utilvserver_getProcEntry(pid, 0, buf, bufsize);
  if (pos==0 && errno==EAGAIN) return false;
  
  SET_TAG_POS("\n__NR_set_ipv4root: ");
  if ( pos!=0 && getNumRevPair(pos, &num, &rev_ipv4root) ) {
    __NR_set_ipv4root_rev0 =
      __NR_set_ipv4root_rev1 =
      __NR_set_ipv4root_rev2 =
      __NR_set_ipv4root_rev3 = num;
  }

  SET_TAG_POS("\n__NR_new_s_context: ");
  if ( pos!=0 && getNumRevPair(pos, &num, &rev_s_context) )
    __NR_new_s_context_rev0 = num;

  return true;
}

#undef SET_TAG_POS

static void init()
{
	if (!is_init){
		__NR_set_ipv4root_rev0 = def_NR_set_ipv4root;
		__NR_set_ipv4root_rev1 = def_NR_set_ipv4root;
		__NR_set_ipv4root_rev2 = def_NR_set_ipv4root;
		__NR_set_ipv4root_rev3 = def_NR_set_ipv4root;
		__NR_new_s_context_rev0 = def_NR_new_s_context;

		while (!init_internal() && errno==EAGAIN) {}

		is_init = true;
	}
}

void vc_init_legacy()
{
        init();
}

void vc_init_internal_legacy(int ctx_rev, int ctx_number,
			     int ipv4_rev, int ipv4_number)
{	
  rev_s_context           = ctx_rev;
  __NR_new_s_context_rev0 = ctx_number;

  rev_ipv4root            = ipv4_rev;
  __NR_set_ipv4root_rev0  = ipv4_number;
  __NR_set_ipv4root_rev1  = ipv4_number;
  __NR_set_ipv4root_rev2  = ipv4_number;
  __NR_set_ipv4root_rev3  = ipv4_number;

  is_init = true;
}

static ALWAYSINLINE xid_t
vc_new_s_context_legacy(int ctx, int remove_cap, int flags)
{
        xid_t ret = -1;
	init();
	if (rev_s_context == 0){
	        return new_s_context_rev0(ctx, remove_cap, flags);
	}else{
		errno = -ENOSYS;
		ret   = VC_NOCTX;
	}
	return ret;
}

static ALWAYSINLINE int
vc_set_ipv4root_legacy_internal (
	unsigned long ip[],
	int nb,
	unsigned long bcast,
	unsigned long mask[])
{
	init();
	if (rev_ipv4root == 0){
		if (nb > 1){
			WRITE_MSG(2,"set_ipv4root: Several IP number specified, but this kernel only supports one. Ignored\n");
		}
		return set_ipv4root_rev0 (ip[0]);
	}else if (rev_ipv4root == 1){
		if (nb > 1){
			WRITE_MSG(2,"set_ipv4root: Several IP number specified, but this kernel only supports one. Ignored\n");
		}
		return set_ipv4root_rev1 (ip[0],bcast);
	}else if (rev_ipv4root == 2){
		return set_ipv4root_rev2 (ip,nb,bcast);
	}else if (rev_ipv4root == 3){
		return set_ipv4root_rev3 (ip,nb,bcast,mask);
	}
	errno = EINVAL;
	return -1;
}

static ALWAYSINLINE int
vc_set_ipv4root_legacy(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips)
{
  unsigned long	ip[nb];
  unsigned long	mask[nb];
  size_t	i;

  for (i=0; i<nb; ++i) {
    ip[i]   = ips[i].ip;
    mask[i] = ips[i].mask;
  }

  return vc_set_ipv4root_legacy_internal(ip, nb, bcast, mask);
}
