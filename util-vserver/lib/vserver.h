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
#include <sys/types.h>

/** the value which is returned in error-case (no ctx found) */
#define VC_NOCTX		((xid_t)(-1))
/** the value which means a random (the next free) ctx */
#define VC_RANDCTX		((xid_t)(-1))
/** the value which means the current ctx */
#define VC_SAMECTX		((xid_t)(-2))

#define VC_LIM_INFINITY		(~0ULL)
#define VC_LIM_KEEP		(~1ULL)

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
     *                ordinary users also */
  int	vc_new_s_context(xid_t ctx, unsigned int remove_cap, unsigned int flags);

    /** Sets the ipv4root information.
     *  \precondition: nb<16 */
  int	vc_set_ipv4root(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips);
  

  /* rlimit related functions */
  typedef uint64_t	vc_limit_t;
  
  
  struct vc_rlimit
  {
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
    
#ifdef __cplusplus
}
#endif

#endif
