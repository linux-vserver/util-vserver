/* $Id$

*  Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
*   
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*   
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*   
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef H_VSERVER_SYSCALL_H
#define H_VSERVER_SYSCALL_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

/** the value which is returned in error-case (no ctx found) */
#define VC_NOCTX		((xid_t)(-1))
/** the value which means a random (the next free) ctx */
#define VC_RANDCTX		((xid_t)(-1))
/** the value which means the current ctx */
#define VC_SAMECTX		((xid_t)(-2))

#define VC_LIM_INFINITY		(~0ULL)
#define VC_LIM_KEEP		(~1ULL)

  
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
#define VC_CAP_QUOTACTL          	29

#define VC_IMMUTABLE_FILE_FL		0x00000010l
#define VC_IMMUTABLE_LINK_FL		0x00008000l
#define VC_IMMUTABLE_ALL		(VC_IMMUTABLE_LINK_FL|VC_IMMUTABLE_FILE_FL)

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
     *  - VC_SAMECTX  which means the current context (just for changing caps and flags)
     *  - VC_RANDCTX  which means the next free context; this value can be used by
     *                ordinary users also
     *  See http://vserver.13thfloor.at/Stuff/Logic.txt for details */
  int	vc_new_s_context(xid_t ctx, unsigned int remove_cap, unsigned int flags);

    /** Sets the ipv4root information.
     *  \precondition: nb<16 */
  int	vc_set_ipv4root(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips);
  
  /* rlimit related functions */
  typedef uint64_t	vc_limit_t;
  
  
  struct vc_rlimit {
      vc_limit_t min;
      vc_limit_t soft;
      vc_limit_t hard;      
  };

  struct  vc_rlimit_mask {
      uint32_t min;
      uint32_t soft;
      uint32_t hard;
  };

  int	vc_get_rlimit(xid_t ctx, int resource, struct vc_rlimit *lim);
  int	vc_set_rlimit(xid_t ctx, int resource, struct vc_rlimit const *lim);
  int	vc_get_rlimit_mask(xid_t ctx, struct vc_rlimit_mask *lim);


  /** sends a signal to a context/pid
      Special values for pid are:
      * -1   which means every process in ctx except the init-process
      *  0   which means every process in ctx inclusive the init-process */
  int	vc_ctx_kill(xid_t ctx, pid_t pid, int sig);


    /** Returns the context of the given process. pid==0 means the current process. */
  xid_t	vc_X_getctx(pid_t pid);

    /** Returns the pid of the 'init' process */
  pid_t	vc_X_getinitpid(pid_t pid);


  xid_t		vc_X_get_filecontext(int fd);
  int		vc_X_set_filecontext(int fd, xid_t ctx);

  int		vc_X_get_ext2flags(int fd, long *flags);
  int		vc_X_set_ext2flags(int fd, long set_flags, long del_flags);


  int		vc_text2cap(char const *);
  char const *	vc_cap2text(int);

  inline static int	vc_get_securecaps() {
    return ( (1<<VC_CAP_LINUX_IMMUTABLE) | (1<<VC_CAP_NET_BROADCAST) |
	     (1<<VC_CAP_NET_ADMIN) | (1<<VC_CAP_NET_RAW) |
	     (1<<VC_CAP_IPC_LOCK) | (1<<VC_CAP_IPC_OWNER) |
	     (1<<VC_CAP_SYS_MODULE) | (1<<VC_CAP_SYS_RAWIO) |
	     (1<<VC_CAP_SYS_PACCT) | (1<<VC_CAP_SYS_ADMIN) |
	     (1<<VC_CAP_SYS_BOOT) | (1<<VC_CAP_SYS_NICE) |
	     (1<<VC_CAP_SYS_RESOURCE) | (1<<VC_CAP_SYS_TIME) |
	     (1<<VC_CAP_MKNOD) | (1<<VC_CAP_QUOTACTL) );
  }

  
  /* The management part */

#define VC_LIMIT_VSERVER_NAME_LEN	1024
  
  typedef enum { vcCFG_NONE, vcCFG_AUTO,
		 vcCFG_LEGACY,
		 vcCFG_RECENT_SHORT,
		 vcCFG_RECENT_FULL }		vcCfgStyle;

  vcCfgStyle	vc_getVserverCfgStyle(char const *id);
  
  /** Resolves the name of the vserver. The result will be allocated and must
      be freed by the caller. */
  char *	vc_getVserverName(char const *id, vcCfgStyle style);

  /** Returns the path of the vserver configuration directory. When the given
   *  vserver does not exist, or when it does not have such a directory, NULL
   *  will be returned. Else, the result will be allocated and must be freed
   *  by the caller. */
  char *	vc_getVserverCfgDir(char const *id, vcCfgStyle style);

  /** Returns the path of the configuration directory for the given
   *  application. The result will be allocated and must be freed by the
   *  caller. */
  char *	vc_getVserverAppDir(char const *id, vcCfgStyle style, char const *app);

  /** Returns the path to the vserver root-directory. The result will be
   *  allocated and must be freed by the caller. */
  char *	vc_getVserverVdir(char const *id, vcCfgStyle style);

  /** Returns the ctx of the given vserver. When vserver is not running and
   *  'honor_static' is false, VC_NOCTX will be returned. Else, when
   *  'honor_static' is true and a static assignment exists, those value will
   *  be returned. Else, the result will be VC_NOCTX.
   *
   *  When 'is_running' is not null, the status of the vserver will be
   *  assigned to this variable. */
  xid_t		vc_getVserverCtx(char const *id, vcCfgStyle style,
				 bool honor_static, bool /*@null@*/ *is_running);

  /** Resolves the cfg-path of the vserver owning the given ctx. 'revdir' will
      be used as the directory holding the mapping-links; when NULL, the
      default value will be assumed.  The result will be allocated and must be
      freed by the caller. */
  char *	vc_getVserverByCtx(xid_t ctx, /*@null@*/vcCfgStyle *style,
				   /*@null@*/char const *revdir);
  
#ifdef __cplusplus
}
#endif

#endif
