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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <syscall.h>
#include <asm/unistd.h>

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


static _syscall1(int, set_ipv4root_rev0, unsigned long, ip)
static _syscall2(int, set_ipv4root_rev1, unsigned long, ip, unsigned long, bcast)
static _syscall3(int, set_ipv4root_rev2, unsigned long *, ip, int, nb, unsigned long, bcast)
static _syscall4(int, set_ipv4root_rev3, unsigned long *, ip, int, nb, unsigned long, bcast, unsigned long *, mask)

static int def_NR_new_s_context = 273;
#undef __NR_new_s_context
static int __NR_new_s_context_rev0;
  //static int __NR_new_s_context_rev1;
static int rev_s_context=0;

static _syscall3(int, new_s_context_rev0, int, newctx, int, remove_cap, int, flags)
    //static _syscall4(int, new_s_context_rev1, int, nbctx, int *, ctxs, int, remove_cap, int, flags)

#if 0
#undef __NR_set_ctxlimit
static int __NR_set_ctxlimit=-1;
static int rev_set_ctxlimit=-1;

static _syscall2 (int, set_ctxlimit, int, resource, long, limit)
#endif

#undef __NR_chrootsafe
static int __NR_chrootsafe=-1;
static int rev_chrootsafe=-1;

static _syscall1 (int, chrootsafe, const char *, dir)
  
static void init()
{
	static int is_init = 0;
	if (!is_init){
		FILE *fin = fopen ("/proc/self/status","r");
		__NR_set_ipv4root_rev0 = def_NR_set_ipv4root;
		__NR_set_ipv4root_rev1 = def_NR_set_ipv4root;
		__NR_set_ipv4root_rev2 = def_NR_set_ipv4root;
		__NR_set_ipv4root_rev3 = def_NR_set_ipv4root;
		__NR_new_s_context_rev0 = def_NR_new_s_context;
		  //__NR_new_s_context_rev1 = def_NR_new_s_context;
		if (fin != NULL){
			char line[100];
			while (fgets(line,sizeof(line)-1,fin)!=NULL){
				int num;
				char title[100],rev[100];
				rev[0] = '\0';
				if (sscanf(line,"%s %d %s",title,&num,rev)>=2){
					if (strcmp(title,"__NR_set_ipv4root:")==0){
						__NR_set_ipv4root_rev0 = num;
						__NR_set_ipv4root_rev1 = num;
						__NR_set_ipv4root_rev2 = num;
						__NR_set_ipv4root_rev3 = num;
						if (strncmp(rev,"rev",3)==0){
							rev_ipv4root = atoi(rev+3);
						}
#if 0						
					}else if (strcmp(title,"__NR_set_ctxlimit:")==0){
						__NR_set_ctxlimit = num;
						if (strncmp(rev,"rev",3)==0){
							rev_set_ctxlimit = atoi(rev+3);
						}
#endif						
					}else if (strcmp(title,"__NR_chrootsafe:")==0){
						__NR_chrootsafe = num;
						if (strncmp(rev,"rev",3)==0){
							rev_chrootsafe = atoi(rev+3);
						}
					}else if (strcmp(title,"__NR_new_s_context:")==0){
						__NR_new_s_context_rev0 = num;
						  //__NR_new_s_context_rev1 = num;
						if (strncmp(rev,"rev",3)==0){
							rev_s_context = atoi(rev+3);
						}
					}
				}
			}
			fclose (fin);
		}
		is_init = 1;
	}
}

void vc_init_legacy()
{
        init();
}

static ALWAYSINLINE int
vc_new_s_context_legacy(int ctx, int remove_cap, int flags)
{
	int ret = -1;
	init();
	if (rev_s_context == 0){
	        return new_s_context_rev0(ctx, remove_cap, flags);
	}else{
		errno = -ENOSYS;
		ret   = -1;
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
			fprintf (stderr,"set_ipv4root: Several IP number specified, but this kernel only supports one. Ignored\n");
		}
		return set_ipv4root_rev0 (ip[0]);
	}else if (rev_ipv4root == 1){
		if (nb > 1){
			fprintf (stderr,"set_ipv4root: Several IP number specified, but this kernel only supports one. Ignored\n");
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
  size_t		i;

  for (i=0; i<nb; ++i) {
    ip[i]   = ips[i].ip;
    mask[i] = ips[i].mask;
  }

  return vc_set_ipv4root_legacy_internal(ip, nb, bcast, mask);
}

static ALWAYSINLINE int
vc_chrootsafe_legacy (const char *dir)
{
	init();
	if (rev_chrootsafe == -1){
	        vc_tell_unsafe_chroot();
		return chroot(dir);
	}else if (rev_chrootsafe == 0){
		return chrootsafe (dir);
	}else{
		fprintf (stderr,"chrootsafe: kernel support version %d, application expects version 0\n"
			,rev_chrootsafe);
	}
	errno = EINVAL;
	return -1;
}

#if 0
/*
	Return != 0 if chrootsafe is available
*/
int has_chrootsafe()
{
	init();
	return rev_chrootsafe != -1;
}

int call_set_ctxlimit (int res, long limit)
{
	init();
	if (rev_set_ctxlimit == -1){
		fprintf (stderr,"set_ctxlimit: Unsupported system call, update kernel\n");
	}else if (rev_set_ctxlimit == 0){
		return set_ctxlimit (res,limit);
	}else{
		fprintf (stderr,"set_ctxlimit: kernel support version %d, application expects version 0\n"
			,rev_set_ctxlimit);
	}
	errno = EINVAL;
	return -1;
}


#endif
