// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#ifndef H_VSERVER_SYSCALL_H
#define H_VSERVER_SYSCALL_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef VC_NOCTX
#  define VC_NOCTX	((ctx_t)(-1))
#endif

#ifndef S_CTX_INFO_LOCK
#  define S_CTX_INFO_LOCK	1
#endif

#ifndef S_CTX_INFO_SCHED
#  define S_CTX_INFO_SCHED	2
#endif

#ifndef S_CTX_INFO_NPROC
#  define S_CTX_INFO_NPROC	4
#endif

#ifndef S_CTX_INFO_PRIVATE
#  define S_CTX_INFO_PRIVATE	8
#endif

#ifndef S_CTX_INFO_INIT
#  define S_CTX_INFO_INIT	16
#endif

#ifndef S_CTX_INFO_HIDEINFO
#  define S_CTX_INFO_HIDEINFO	32
#endif

#ifndef S_CTX_INFO_ULIMIT
#  define S_CTX_INFO_ULIMIT	64
#endif

#define VC_CAP_CHOWN            	 0
#define VC_CAP_DAC_OVERRIDE     	 1
#define VC_CAP_DAC_READ_SEARCH  	 2
#define VC_CAP_FOWNER           	 3
#define VC_CAP_FSETID           	 4
#define VC_CAP_KILL             	 5
#define VC_CAP_SETGID           	 6
#define VC_CAP_SETUID           	 7
#define VC_CAP_SETPCAP          	 8
#define VC_CAP_LINUX_IMMUTABLE  	 9
#define VC_CAP_NET_BIND_SERVICE 	10
#define VC_CAP_NET_BROADCAST    	11
#define VC_CAP_NET_ADMIN        	12
#define VC_CAP_NET_RAW          	13
#define VC_CAP_IPC_LOCK         	14
#define VC_CAP_IPC_OWNER        	15
#define VC_CAP_SYS_MODULE       	16
#define VC_CAP_SYS_RAWIO        	17
#define VC_CAP_SYS_CHROOT       	18
#define VC_CAP_SYS_PTRACE       	19
#define VC_CAP_SYS_PACCT        	20
#define VC_CAP_SYS_ADMIN        	21
#define VC_CAP_SYS_BOOT         	22
#define VC_CAP_SYS_NICE         	23
#define VC_CAP_SYS_RESOURCE     	24
#define VC_CAP_SYS_TIME 		25
#define VC_CAP_SYS_TTY_CONFIG   	26
#define VC_CAP_MKNOD            	27
#define VC_CAP_LEASE            	28
#define VC_CAP_OPENDEV          	29


#ifdef __cplusplus
extern "C" {
#endif

  struct vc_ip_mask_pair {
    uint32_t	ip;
    uint32_t	mask;
  };

    /** Returns version of the given API-category */
  int	vc_get_version(int category);
  
    /** Puts current process into context <ctx>, removes the given caps and
     *  sets flags.
     *  Special values for ctx are
     *  - -2 which means the current context (just for changing caps and flags)
     *  - -1 which means the next free context; this value can be used by
     *    ordinary users also */
  int	vc_new_s_context(ctx_t ctx, unsigned int remove_cap, unsigned int flags);

    /** Sets the ipv4root information.
     *  \precondition: nb<16 */
  int	vc_set_ipv4root(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips);
  
  int	vc_chrootsafe(char const *dir);


    /** Returns the context of the given process. pid==0 means the current process. */
  ctx_t	vc_X_getctx(pid_t pid);

    /** Returns the pid of the 'init' process */
  pid_t	vc_X_getinitpid(pid_t pid);


  int		vc_text2cap(char const *);
  char const *	vc_cap2text(int);
  
#ifdef __cplusplus
}
#endif

#endif
