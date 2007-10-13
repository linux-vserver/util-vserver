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

/** \file vserver.h
 *  \brief The public interface of the the libvserver library.
 */

#ifndef H_VSERVER_SYSCALL_H
#define H_VSERVER_SYSCALL_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sched.h>
#include <netinet/in.h>

#ifndef IS_DOXYGEN
#if defined(__GNUC__)
#  define VC_ATTR_UNUSED                __attribute__((__unused__))
#  define VC_ATTR_NORETURN              __attribute__((__noreturn__))
#  define VC_ATTR_CONST			__attribute__((__const__))
#  define VC_ATTR_DEPRECATED		__attribute__((__deprecated__))
#  if __GNUC__*0x10000 + __GNUC_MINOR__*0x100 + __GNUC_PATCHLEVEL__ >= 0x30300
#    define VC_ATTR_NONNULL(ARGS)	__attribute__((__nonnull__ ARGS))
#    define VC_ATTR_ALWAYSINLINE        __attribute__((__always_inline__))
#  else
#    define VC_ATTR_NONNULL(ARGS)
#    define VC_ATTR_ALWAYSINLINE
#  endif
#  if __GNUC__*0x10000 + __GNUC_MINOR__*0x100 + __GNUC_PATCHLEVEL__ >= 0x30303
#    define VC_ATTR_PURE		__attribute__((__pure__))
#  else
#    define VC_ATTR_PURE
#  endif
#else
#  define VC_ATTR_NONNULL(ARGS)
#  define VC_ATTR_UNUSED
#  define VC_ATTR_NORETURN
#  define VC_ATTR_ALWAYSINLINE
#  define VC_ATTR_DEPRECATED
#  define VC_ATTR_PURE
#  define VC_ATTR_CONST
#endif
#endif	// IS_DOXYGEN

/** the value which is returned in error-case (no ctx found) */
#define VC_NOCTX		((xid_t)(-1))
#define VC_NOXID		((xid_t)(-1))
/** the value which means a random (the next free) ctx */
#define VC_DYNAMIC_XID		((xid_t)(-1))
/** the value which means the current ctx */
#define VC_SAMECTX		((xid_t)(-2))

#define VC_NONID		((nid_t)(-1))
#define VC_DYNAMIC_NID		((nid_t)(-1))

#define VC_LIM_INFINITY		(~0ULL)
#define VC_LIM_KEEP		(~1ULL)

#define VC_CDLIM_UNSET		(0U)
#define VC_CDLIM_INFINITY	(~0U)
#define VC_CDLIM_KEEP		(~1U)
  
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

#ifndef S_CTX_INFO_NAMESPACE
#  define S_CTX_INFO_NAMESPACE	128
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
#define VC_CAP_AUDIT_WRITE          	29
#define VC_CAP_AUDIT_CONTROL          	30

#define VC_IMMUTABLE_FILE_FL		0x0000010lu
#define VC_IMMUTABLE_LINK_FL		0x0008000lu
#define VC_IMMUTABLE_ALL		(VC_IMMUTABLE_LINK_FL|VC_IMMUTABLE_FILE_FL)

#define VC_IATTR_XID			0x01000000u

#define VC_IATTR_ADMIN			0x00000001u
#define VC_IATTR_WATCH			0x00000002u
#define VC_IATTR_HIDE			0x00000004u
#define VC_IATTR_FLAGS			0x00000007u

#define VC_IATTR_BARRIER		0x00010000u
#define	VC_IATTR_IUNLINK		0x00020000u
#define VC_IATTR_IMMUTABLE		0x00040000u


// the flags
#define VC_VXF_INFO_LOCK		0x00000001ull
#define VC_VXF_INFO_NPROC		0x00000004ull
#define VC_VXF_INFO_PRIVATE		0x00000008ull
#define VC_VXF_INFO_INIT		0x00000010ull

#define VC_VXF_INFO_HIDEINFO		0x00000020ull
#define VC_VXF_INFO_ULIMIT		0x00000040ull
#define VC_VXF_INFO_NAMESPACE		0x00000080ull

#define	VC_VXF_SCHED_HARD		0x00000100ull
#define	VC_VXF_SCHED_PRIO		0x00000200ull
#define	VC_VXF_SCHED_PAUSE		0x00000400ull

#define VC_VXF_VIRT_MEM			0x00010000ull
#define VC_VXF_VIRT_UPTIME		0x00020000ull
#define VC_VXF_VIRT_CPU			0x00040000ull
#define VC_VXF_VIRT_LOAD		0x00080000ull
#define VC_VXF_VIRT_TIME		0x00100000ull

#define VC_VXF_HIDE_MOUNT		0x01000000ull
#define VC_VXF_HIDE_NETIF		0x02000000ull
#define VC_VXF_HIDE_VINFO		0x04000000ull

#define	VC_VXF_STATE_SETUP		(1ULL<<32)
#define	VC_VXF_STATE_INIT		(1ULL<<33)
#define VC_VXF_STATE_ADMIN		(1ULL<<34)

#define VC_VXF_SC_HELPER		(1ULL<<36)
#define VC_VXF_REBOOT_KILL		(1ULL<<37)
#define VC_VXF_PERSISTENT		(1ULL<<38)

#define VC_VXF_FORK_RSS			(1ULL<<48)
#define VC_VXF_PROLIFIC			(1ULL<<49)

#define VC_VXF_IGNEG_NICE		(1ULL<<52)


// the ccapabilities
#define VC_VXC_SET_UTSNAME		0x00000001ull
#define VC_VXC_SET_RLIMIT		0x00000002ull

#define VC_VXC_RAW_ICMP			0x00000100ull
#define VC_VXC_SYSLOG			0x00001000ull

#define VC_VXC_SECURE_MOUNT		0x00010000ull
#define VC_VXC_SECURE_REMOUNT		0x00020000ull
#define VC_VXC_BINARY_MOUNT		0x00040000ull

#define VC_VXC_QUOTA_CTL		0x00100000ull
#define VC_VXC_ADMIN_MAPPER		0x00200000ull
#define VC_VXC_ADMIN_CLOOP		0x00400000ull


// the scheduler flags
#define VC_VXSM_FILL_RATE		0x0001
#define VC_VXSM_INTERVAL		0x0002
#define VC_VXSM_FILL_RATE2		0x0004
#define VC_VXSM_INTERVAL2		0x0008
#define VC_VXSM_TOKENS			0x0010
#define VC_VXSM_TOKENS_MIN		0x0020
#define VC_VXSM_TOKENS_MAX		0x0040
#define VC_VXSM_PRIO_BIAS		0x0100
#define VC_VXSM_CPU_ID			0x1000
#define VC_VXSM_BUCKET_ID		0x2000

#define VC_VXSM_IDLE_TIME		0x0200
#define VC_VXSM_FORCE			0x0400
#define VC_VXSM_MSEC			0x4000

#define VC_VXSM_V3_MASK			0x0173


// the network flags
#define VC_NXF_INFO_LOCK		0x00000001ull
#define VC_NXF_INFO_PRIVATE		0x00000008ull

#define VC_NXF_SINGLE_IP		0x00000100ull
#define VC_NXF_LBACK_REMAP		0x00000200ull

#define VC_NXF_HIDE_NETIF		0x02000000ull
#define VC_NXF_HIDE_LBACK		0x04000000ull

#define VC_NXF_STATE_SETUP		(1ULL<<32)
#define VC_NXF_STATE_ADMIN		(1ULL<<34)

#define VC_NXF_SC_HELPER		(1ULL<<36)
#define VC_NXF_PERSISTENT		(1ULL<<38)


// the network capabilities
#define VC_NXC_RAW_ICMP			0x00000100ull


// the vserver specific limits
#define VC_VLIMIT_NSOCK			16
#define VC_VLIMIT_OPENFD		17
#define VC_VLIMIT_ANON			18
#define VC_VLIMIT_SHMEM			19
#define VC_VLIMIT_SEMARY		20
#define VC_VLIMIT_NSEMS			21
#define VC_VLIMIT_DENTRY		22
#define VC_VLIMIT_MAPPED		23


// the VCI bit values
#define VC_VCI_NO_DYNAMIC		(1 << 0)
#define VC_VCI_SPACES			(1 << 10)
#define VC_VCI_NETV2			(1 << 11)
#define VC_VCI_PPTAG			(1 << 28)


// the device mapping flags
#define VC_DATTR_CREATE			0x00000001
#define VC_DATTR_OPEN			0x00000002

#define VC_DATTR_REMAP			0x00000010


// the process context migration flags
#define VC_VXM_SET_INIT			0x00000001
#define VC_VXM_SET_REAPER		0x00000002


// the network address flags
#define VC_NXA_TYPE_IPV4		0x0001
#define VC_NXA_TYPE_IPV6		0x0002

#define VC_NXA_TYPE_NONE		0x0000
#define VC_NXA_TYPE_ANY			0x00FF

#define VC_NXA_TYPE_ADDR		0x0010
#define VC_NXA_TYPE_MASK		0x0020
#define VC_NXA_TYPE_RANGE		0x0040

#define VC_NXA_MOD_BCAST		0x0100
#define VC_NXA_MOD_LBACK		0x0200


#ifndef CLONE_NEWNS
#  define CLONE_NEWNS			0x00020000
#endif
#ifndef CLONE_NEWUTS
#  define CLONE_NEWUTS			0x04000000
#endif
#ifndef CLONE_NEWIPC
#  define CLONE_NEWIPC			0x08000000
#endif
#ifndef CLONE_NEWUSER
#  define CLONE_NEWUSER			0x10000000
#endif


#define VC_BAD_PERSONALITY		((uint_least32_t)(-1))


/** \defgroup  syscalls Syscall wrappers
 *  Functions which are calling the vserver syscall directly. */

/** \defgroup  helper   Helper functions
 *  Functions which are doing general helper tasks like parameter parsing. */

/** \typedef  an_unsigned_integer_type  xid_t
 *  The identifier of a context. */

#ifdef IS_DOXYGEN
typedef an_unsigned_integer_type	xid_t;
typedef an_unsigned_integer_type	nid_t;
typedef an_unsigned_integer_type	tag_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /** \brief   The generic vserver syscall
     *  \ingroup syscalls
     *
     *  This function executes the generic vserver syscall. It uses the
     *  correct syscallnumber (which may differ between the different
     *  architectures).
     *
     *  \param   cmd  the command to be executed
     *  \param   xid  the xid on which the cmd shall be applied
     *  \param   data additional arguments; depends on \c cmd
     *  \returns depends on \c cmd; usually, -1 stands for an error
     */
  int		vc_syscall(uint32_t cmd, xid_t xid, void *data);

    /** \brief   Returns the version of the current kernel API.
     *  \ingroup syscalls
     *	\returns The versionnumber of the kernel API
     */
  int		vc_get_version();

  typedef	uint64_t vc_vci_t;
    /** \brief   Returns the kernel configuration bits
     *  \ingroup syscalls
     *  \returns The kernel configuration bits
     */
  vc_vci_t	vc_get_vci();

    /** \brief   Moves current process into a context
     *  \ingroup syscalls
     *
     *  Puts current process into context \a ctx, removes the capabilities
     *  given in \a remove_cap and sets \a flags.
     *
     *  \param ctx         The new context; special values for are
     *  - VC_SAMECTX      which means the current context (just for changing caps and flags)
     *  - VC_DYNAMIC_XID  which means the next free context; this value can be used by
     *                    ordinary users also
     *  \param remove_cap  The linux capabilities which will be \b removed.
     *  \param flags       Special flags which will be set.
     *
     *  \returns  The new context-id, or VC_NOCTX on errors; \c errno
     *	          will be set appropriately
     *
     *  See http://vserver.13thfloor.at/Stuff/Logic.txt for details */
  xid_t		vc_new_s_context(xid_t ctx, unsigned int remove_cap, unsigned int flags);

  struct vc_ip_mask_pair {
      uint32_t	ip;
      uint32_t	mask;
  };

    /** \brief  Sets the ipv4root information.
     *  \ingroup syscalls
     *  \pre    \a nb < NB_IPV4ROOT && \a ips != 0 */
  int		vc_set_ipv4root(uint32_t  bcast, size_t nb,
			struct vc_ip_mask_pair const *ips) VC_ATTR_NONNULL((3));

    /** \brief  Returns the value of NB_IPV4ROOT.
     *  \ingroup helper
     *
     *  This function returns the value of NB_IPV4ROOT which was used when the
     *  library was built, but \b not the value which is used by the currently
     *  running kernel. */
  size_t	vc_get_nb_ipv4root() VC_ATTR_CONST VC_ATTR_PURE;

    /* process context */
  /** \brief    Flags of process-contexts
   */
  struct  vc_ctx_flags {
      /** \brief Mask of set context flags */
      uint_least64_t	flagword;
      /** \brief Mask of set and unset context flags when used by set
       *         operations, or modifiable flags when used by get
       *         operations */
      uint_least64_t	mask;
  };

    /** \brief   Creates a context without starting it.
     *  \ingroup syscalls
     *
     *  This functions initializes a new context. When already in a freshly
     *  created context, this old context will be discarded.
     *
     *  \param xid  The new context; special values are:
     *	- VC_DYNAMIC_XID which means to create a dynamic context
     *
     *	\returns the xid of the created context, or VC_NOCTX on errors. \c errno
     *	         will be set appropriately. */
  xid_t		vc_ctx_create(xid_t xid, struct vc_ctx_flags *flags);

    /** \brief   Moves the current process into the specified context.
     *  \ingroup syscalls
     *
     *  \param   xid    The new context
     *  \param   flags  The flags, see VC_VXM_*
     *  \returns 0 on success, -1 on errors */
  int		vc_ctx_migrate(xid_t xid, uint_least64_t flags);

    /** \brief   Statistics about a context */
  struct vc_ctx_stat {
      uint_least32_t	usecnt;	///< number of uses
      uint_least32_t	tasks;	///< number of tasks
  };

    /** \brief   Get some statistics about a context.
     *  \ingroup syscalls
     *
     *  \param   xid   The context to get stats about
     *  \param   stat  Where to store the result
     *
     *  \returns 0 on success, -1 on errors. */
  int		vc_ctx_stat(xid_t xid, struct vc_ctx_stat /*@out@*/ *stat) VC_ATTR_NONNULL((2));

    /** \brief   Contains further statistics about a context. */
  struct vc_virt_stat {
      uint_least64_t	offset;
      uint_least64_t	uptime;
      uint_least32_t	nr_threads;
      uint_least32_t	nr_running;
      uint_least32_t	nr_uninterruptible;
      uint_least32_t	nr_onhold;
      uint_least32_t	nr_forks;
      uint_least32_t	load[3];
  };

    /** \brief   Get more statistics about a context.
     *  \ingroup syscalls
     *
     *  \param xid   The context to get stats about
     *  \param stat  Where to store the result
     *
     *  \returns 0 on success, -1 on errors. */
  int		vc_virt_stat(xid_t xid, struct vc_virt_stat /*@out@*/ *stat) VC_ATTR_NONNULL((2));

  /** \brief    Sends a signal to a context/pid
   *  \ingroup  syscalls
   *
   *  Special values for \a pid are:
   *  - -1   which means every process in ctx except the init-process
   *  -  0   which means every process in ctx inclusive the init-process */
  int		vc_ctx_kill(xid_t ctx, pid_t pid, int sig);
 
  int		vc_get_cflags(xid_t xid, struct vc_ctx_flags /*@out@*/ *)	VC_ATTR_NONNULL((2));
  int		vc_set_cflags(xid_t xid, struct vc_ctx_flags /*@in@*/ const *)	VC_ATTR_NONNULL((2));

  /** \brief    Capabilities of process-contexts */
  struct  vc_ctx_caps {
      /** \brief  Mask of set common system capabilities */
      uint_least64_t	bcaps;
      /** \brief Mask of set and unset common system capabilities when used by
       *         set operations, or the modifiable capabilities when used by
       *         get operations */
      uint_least64_t	bmask;
      /** \brief Mask of set process context capabilities */
      uint_least64_t	ccaps;
      /** \brief Mask of set and unset process context capabilities when used
       *         by set operations, or the modifiable capabilities when used
       *         by get operations */
      uint_least64_t	cmask;
  };

  int		vc_get_ccaps(xid_t xid, struct vc_ctx_caps *);
  int		vc_set_ccaps(xid_t xid, struct vc_ctx_caps const *);

  struct vc_vx_info {
      xid_t	xid;
      pid_t	initpid;
  };

  int		vc_get_vx_info(xid_t xid, struct vc_vx_info *info) VC_ATTR_NONNULL((2));
  
    /** \brief   Returns the context of the given process.
     *  \ingroup syscalls
     *
     *  \param  pid  the process-id whose xid shall be determined;
     *               pid==0 means the current process.
     *  \returns     the xid of process \c pid or -1 on errors
     */
  xid_t		vc_get_task_xid(pid_t pid);

  /** \brief   Waits for the end of a context
   *  \ingroup syscalls
   */
  int		vc_wait_exit(xid_t xid);
  
    /* rlimit related functions */
  
    /** \brief  The type which is used for a single limit value.
     *
     *  Special values are
     *  - VC_LIM_INFINITY ... which is the infinite value
     *  - VC_LIM_KEEP     ... which is used to mark values which shall not be
     *                        modified by the vc_set_rlimit() operation.
     *
     *  Else, the interpretation of the value depends on the corresponding
     *  resource; it might be bytes, pages, seconds or litres of beer. */
  typedef uint_least64_t	vc_limit_t;

    /** \brief  Masks describing the supported limits. */
  struct  vc_rlimit_mask {
      uint_least32_t	min;	///< masks the resources supporting a minimum limit
      uint_least32_t	soft;	///< masks the resources supporting a soft limit
      uint_least32_t	hard;	///< masks the resources supporting a hard limit
  };

    /** \brief  Returns the limits supported by the kernel */
  int		vc_get_rlimit_mask(xid_t xid,
			   struct vc_rlimit_mask /*@out@*/ *lim) VC_ATTR_NONNULL((2));

    /** \brief  The limits of a resources.
     *
     *  This is a triple consisting of a minimum, soft and hardlimit. */
  struct vc_rlimit {
      vc_limit_t	min;	///< the guaranted minimum of a resources
      vc_limit_t	soft;	///< the softlimit of a resource
      vc_limit_t	hard;	///< the absolute hardlimit of a resource
  };

    /** \brief   Returns the limits of \a resource.
     *  \ingroup syscalls
     *
     *  \param  xid       The id of the context
     *  \param  resource  The resource which will be queried
     *  \param  lim       The result which will be filled with the limits
     *
     *  \returns 0 on success, and -1 on errors. */
  int		vc_get_rlimit(xid_t xid, int resource,
		      struct vc_rlimit       /*@out@*/ *lim) VC_ATTR_NONNULL((3));
    /** \brief   Sets the limits of \a resource.
     *  \ingroup syscalls
     *
     *  \param  xid       The id of the context
     *  \param  resource  The resource which will be queried
     *  \param  lim       The new limits
     *
     *  \returns 0 on success, and -1 on errors. */
  int		vc_set_rlimit(xid_t xid, int resource,
		      struct vc_rlimit const /*@in@*/  *lim) VC_ATTR_NONNULL((3));

    /** \brief Statistics for a resource limit. */
  struct  vc_rlimit_stat {
      uint_least32_t	hits;	 ///< number of hits on the limit
      vc_limit_t	value;	 ///< current value
      vc_limit_t	minimum; ///< minimum value observed
      vc_limit_t	maximum; ///< maximum value observed
  };

    /** \brief   Returns the current stats of \a resource.
     *  \ingroup syscalls
     *
     *  \param  xid       The id of the context
     *  \param  resource  The resource which will be queried
     *  \param  stat      The result which will be filled with the stats
     *
     *  \returns 0 on success, and -1 on errors. */
  int		vc_rlimit_stat(xid_t xid, int resource,
		       struct vc_rlimit_stat /*@out@*/ *stat) VC_ATTR_NONNULL((3));

    /** \brief   Resets the minimum and maximum observed values of all resources.
     *  \ingroup syscalls
     *
     *  \param xid	The id of the context
     *
     *  \returns 0 on success, and -1 on errors. */
  int		vc_reset_minmax(xid_t xid);

    /** \brief   Parses a string describing a limit
     *  \ingroup helper
     *
     *  This function parses \a str and interprets special words like \p "inf"
     *  or suffixes. Valid suffixes are
     *  - \p k ... 1000
     *  - \p m ... 1000000
     *  - \p K ... 1024
     *  - \p M ... 1048576
     *
     *  \param str  The string which shall be parsed
     *  \param res  Will be filled with the interpreted value; in errorcase,
     *              this value is undefined.
     *
     *  \returns \a true, iff the string \a str could be parsed. \a res will
     *  be filled with the interpreted value in this case. 
     *
     *  \pre \a str!=0 && \a res!=0
     */
  bool		vc_parseLimit(char const /*@in@*/ *str, vc_limit_t /*@out@*/ *res)	VC_ATTR_NONNULL((1,2));


    /* network context */
  struct vc_nx_info {
      nid_t	nid;
  };

  nid_t		vc_get_task_nid(pid_t pid);
  int		vc_get_nx_info(nid_t nid, struct vc_nx_info *) VC_ATTR_NONNULL((2));

  struct vc_net_addr {
      uint16_t			vna_type;
      uint16_t			vna_flags;
      uint16_t			vna_prefix;
      uint16_t			vna_parent;
      struct {
	union {
	  struct in_addr	v4;
	  struct in6_addr	v6;
	} ip;
	union {
	  struct in_addr	v4;
	  struct in6_addr	v6;
	} ip2;
	union {
	  struct in_addr	v4;
	  struct in6_addr	v6;
	} mask;
      } s;
#define vna_v4_ip	s.ip.v4
#define vna_v4_ip2	s.ip2.v6
#define vna_v4_mask	s.mask.v4
#define vna_v6_ip	s.ip.v6
#define vna_v6_ip2	s.ip2.v6
#define vna_v6_mask	s.mask.v6
  };

  struct vc_net_flags {
      uint_least64_t	flagword;
      uint_least64_t	mask;
  };

  nid_t		vc_net_create(nid_t nid);
  int		vc_net_migrate(nid_t nid);

  int		vc_net_add(nid_t nid, struct vc_net_addr const *info);
  int		vc_net_remove(nid_t nid, struct vc_net_addr const *info);

  int		vc_get_nflags(nid_t, struct vc_net_flags *);
  int		vc_set_nflags(nid_t, struct vc_net_flags const *);

  struct vc_net_caps {
      uint_least64_t	ncaps;
      uint_least64_t	cmask;
  };

  int		vc_get_ncaps(nid_t, struct vc_net_caps *);
  int		vc_set_ncaps(nid_t, struct vc_net_caps const *);


    /* iattr related functions */

  int		vc_set_iattr(char const *filename, xid_t xid,
			     uint_least32_t flags, uint_least32_t mask) VC_ATTR_NONNULL((1));

  int		vc_fset_iattr(int fd, xid_t xid,
			      uint_least32_t flags, uint_least32_t mask);

    /** \brief   Returns information about attributes and assigned context of a file.
     *  \ingroup syscalls
     *
     *  This function returns the VC_IATTR_XXX flags and about the assigned
     *  context of a file. To request an information, the appropriate bit in
     *  \c mask must be set and the corresponding parameter (\a xid or \a
     *  flags) must not be NULL.
     *
     *  E.g. to receive the assigned context, the \c VC_IATTR_XID bit must be
     *  set in \a mask, and \a xid must point to valid memory.
     *
     *  Possible flags are \c VC_IATTR_ADMIN, \c VC_IATTR_WATCH , \c VC_IATTR_HIDE,
     *  \c VC_IATTR_BARRIER, \c VC_IATTR_IUNLINK and \c VC_IATTR_IMMUTABLE.
     *
     *  \param filename  The name of the file whose attributes shall be determined.

     *  \param xid       When non-zero and the VC_IATTR_XID bit is set in \a mask,
     *                   the assigned context of \a filename will be stored there.
     *  \param flags     When non-zero, a bitmask of current attributes will be
     *                   stored there. These attributes must be requested explicitly
     *                   by setting the appropriate bit in \a mask
     *  \param mask      Points to a bitmask which tells which attributes shall be
     *                   determined. On return, it will masquerade the attributes
     *                   which were determined.
     *
     *  \pre  mask!=0 && !((*mask&VC_IATTR_XID) && xid==0) && !((*mask&~VC_IATTR_XID) && flags==0) */
  int		vc_get_iattr(char const *filename, xid_t * /*@null@*/ xid,
			     uint_least32_t * /*@null@*/ flags,
			     uint_least32_t * /*@null@*/ mask) VC_ATTR_NONNULL((1));

  int		vc_fget_iattr(int fd, xid_t * /*@null@*/ xid,
			      uint_least32_t * /*@null@*/ flags,
			      uint_least32_t * /*@null@*/ mask) VC_ATTR_NONNULL((4));
  
  /** \brief   Returns the context of \c filename
   *  \ingroup syscalls
   *
   *  This function calls vc_get_iattr() with appropriate arguments to
   *  determine the context of \c filename. In error-case or when no context
   *  is assigned, \c VC_NOCTX will be returned. To differ between both cases,
   *  \c errno must be examined.
   *
   *  \b WARNING: this function can modify \c errno although no error happened.
   *
   *  \param   filename  The file to check
   *  \returns The assigned context, or VC_NOCTX when an error occured or no
   *           such assignment exists. \c errno will be 0 in the latter case */
  xid_t		vc_getfilecontext(char const *filename) VC_ATTR_NONNULL((1));


    /* vhi related functions */
  typedef enum { vcVHI_CONTEXT, vcVHI_SYSNAME, vcVHI_NODENAME,
		 vcVHI_RELEASE, vcVHI_VERSION, vcVHI_MACHINE,
		 vcVHI_DOMAINNAME }		vc_uts_type;
  
  int		vc_set_vhi_name(xid_t xid, vc_uts_type type,
				char const *val, size_t len) VC_ATTR_NONNULL((3));
  int		vc_get_vhi_name(xid_t xid, vc_uts_type type,
				char *val, size_t len)       VC_ATTR_NONNULL((3));

    /* namespace related functions */
  int		vc_enter_namespace(xid_t xid, uint_least64_t mask);
  int		vc_set_namespace(xid_t xid, uint_least64_t mask);
  int		vc_cleanup_namespace();
  uint_least64_t vc_get_space_mask();


    /* disk limit related things */
  struct vc_ctx_dlimit {
      uint_least32_t	space_used;
      uint_least32_t	space_total;
      uint_least32_t	inodes_used;
      uint_least32_t	inodes_total;
      uint_least32_t	reserved;
  };
  

  /** Add a disk limit to a file system. */
  int		vc_add_dlimit(char const *filename, xid_t xid,
			      uint_least32_t flags) VC_ATTR_NONNULL((1));
  /** Remove a disk limit from a file system. */
  int		vc_rem_dlimit(char const *filename, xid_t xid,
			      uint_least32_t flags) VC_ATTR_NONNULL((1));

  /** Set a disk limit. */
  int		vc_set_dlimit(char const *filename, xid_t xid,
			      uint_least32_t flags,
			      struct vc_ctx_dlimit const *limits) VC_ATTR_NONNULL((1,4));
  /** Get a disk limit. */
  int		vc_get_dlimit(char const *filename, xid_t xid,
			      uint_least32_t flags,
			      struct vc_ctx_dlimit *limits) VC_ATTR_NONNULL((1));

  /** Get the filesystem tag for a process. */
  tag_t		vc_get_task_tag(pid_t pid);

  /** Create a new filesystem tag space. */
  int		vc_tag_create(tag_t tag);

  /** Migrate to an existing filesystem tag space. */
  int		vc_tag_migrate(tag_t tag);

    /* scheduler related syscalls */
  struct vc_set_sched {
      uint_least32_t	set_mask;
      int_least32_t	fill_rate;
      int_least32_t	interval;
      int_least32_t	fill_rate2;
      int_least32_t	interval2;
      int_least32_t	tokens;
      int_least32_t	tokens_min;
      int_least32_t	tokens_max;
      int_least32_t	priority_bias;
      int_least32_t	cpu_id;
      int_least32_t	bucket_id;
  };

  int		vc_set_sched(xid_t xid, struct vc_set_sched const *) VC_ATTR_NONNULL((2));
  int		vc_get_sched(xid_t xid, struct vc_set_sched *) VC_ATTR_NONNULL((2));

  struct vc_sched_info {
      int_least32_t	cpu_id;
      int_least32_t	bucket_id;
      uint_least64_t	user_msec;
      uint_least64_t	sys_msec;
      uint_least64_t	hold_msec;
      uint_least32_t	token_usec;
      int_least32_t	vavavoom;
  };

  int		vc_sched_info(xid_t xid, struct vc_sched_info *info) VC_ATTR_NONNULL((2));

    /* misc. syscalls */
  int		vc_set_mapping(xid_t xid, const char *device, const char *target, uint32_t flags);
  int		vc_unset_mapping(xid_t xid, const char *device, const char *target, uint32_t flags);

  int		vc_get_badness(xid_t xid, int64_t *badness);
  int		vc_set_badness(xid_t xid, int64_t badness);


  /** \brief    Information about parsing errors
   *  \ingroup  helper
   */
  struct vc_err_listparser {
      char const	*ptr;		///< Pointer to the first character of an erroneous string
      size_t		len;		///< Length of the erroneous string
  };

  /** \brief   Converts a single string into bcapability
   *  \ingroup helper
   *
   *  \param   str   The string to be parsed;
   *                 both "CAP_xxx" and "xxx" will be accepted
   *  \param   len   The length of the string, or \c 0 for automatic detection
   *
   *  \returns 0 on error; a bitmask on success
   *  \pre     \a str != 0
   */
  uint_least64_t	vc_text2bcap(char const *str, size_t len);

  /** \brief   Converts the lowest bit of a bcapability or the entire value
   *           (when possible) to a textual representation
   *  \ingroup helper
   *
   *  \param   val  The string to be converted; on success, the detected bit(s)
   *                will be unset, in errorcase only the lowest set bit
   *
   *  \returns A textual representation of \a val resp. of its lowest set bit;
   *           or \c NULL in errorcase.
   *  \pre     \a val!=0
   *  \post    \a *val<sub>old</sub> \c != 0  \c <-->
   *               \a *val<sub>old</sub> > \a *val<sub>new</sub>
   *  \post    \a *val<sub>old</sub> \c == 0  \c --->  \a result == 0
   */
  char const *	vc_lobcap2text(uint_least64_t *val) VC_ATTR_NONNULL((1));

  /** \brief   Converts a string into a bcapability-bitmask
   *  \ingroup helper
   *
   *  Syntax of \a str: \verbinclude list2xxx.syntax
   *
   *  When the \c `~' prefix is used, the bits will be unset and a `~' after
   *  another `~' will cancel both ones. The \c `^' prefix specifies a
   *  bitnumber instead of a bitmask.
   *
   *  "literal name" is everything which will be accepted by the
   *  vc_text2bcap() function. The special values for \c NAME will be
   *  recognized case insensitively
   *
   *  \param  str   The string to be parsed
   *  \param  len   The length of the string, or \c 0 for automatic detection
   *  \param  err   Pointer to a structure for error-information, or \c NULL.
   *  \param  cap   Pointer to a vc_ctx_caps structure holding the results;
   *                only the \a bcaps and \a bmask fields will be changed and
   *                already set values will not be honored. When an error
   *                occured, \a cap will have the value of all processed valid
   *                \c BCAP parts.
   *
   *  \returns 0 on success, -1 on error. In error case, \a err will hold
   *           position and length of the first not understood BCAP part
   *  \pre     \a str != 0 && \a cap != 0;
   *           \a cap->bcaps and \a cap->bmask must be initialized
   */
  int			vc_list2bcap(char const *str, size_t len,
				     struct vc_err_listparser *err,
				     struct vc_ctx_caps *cap) VC_ATTR_NONNULL((1,4));

  uint_least64_t	vc_text2ccap(char const *, size_t len);
  char const *		vc_loccap2text(uint_least64_t *);
  int			vc_list2ccap(char const *, size_t len,
				     struct vc_err_listparser *err,
				     struct vc_ctx_caps *);

  int			vc_list2cflag(char const *, size_t len,
				     struct vc_err_listparser *err,
				     struct vc_ctx_flags *flags);
  uint_least64_t	vc_text2cflag(char const *, size_t len);
  char const *		vc_locflag2text(uint_least64_t *);
  
  uint_least32_t	vc_list2cflag_compat(char const *, size_t len,
					    struct vc_err_listparser *err);
  uint_least32_t	vc_text2cflag_compat(char const *, size_t len);
  char const *		vc_hicflag2text_compat(uint_least32_t);

  int			vc_text2cap(char const *);
  char const *		vc_cap2text(unsigned int);

  
  int			vc_list2nflag(char const *, size_t len,
				     struct vc_err_listparser *err,
				     struct vc_net_flags *flags);
  uint_least64_t	vc_text2nflag(char const *, size_t len);
  char const *		vc_lonflag2text(uint_least64_t *);

  uint_least64_t	vc_text2ncap(char const *, size_t len);
  char const *		vc_loncap2text(uint_least64_t *);
  int			vc_list2ncap(char const *, size_t len,
				     struct vc_err_listparser *err,
				     struct vc_net_caps *);

  uint_least64_t		vc_get_insecurebcaps() VC_ATTR_CONST;
  inline static uint_least64_t	vc_get_insecureccaps() {
    return ~(VC_VXC_SET_UTSNAME|VC_VXC_RAW_ICMP);
  }
  
  inline static int	vc_setfilecontext(char const *filename, xid_t xid) {
    return vc_set_iattr(filename, xid, 0, VC_IATTR_XID);
  }


  uint_least32_t	vc_text2personalityflag(char const *str,
						size_t len) VC_ATTR_NONNULL((1));

  char const *		vc_lopersonality2text(uint_least32_t *) VC_ATTR_NONNULL((1));
  
  int			vc_list2personalityflag(char const /*@in@*/ *,
						size_t len,
						uint_least32_t /*@out@*/ *personality,
						struct vc_err_listparser /*@out@*/ *err) VC_ATTR_NONNULL((1,3));

  uint_least32_t	vc_str2personalitytype(char const /*@in@*/*,
					       size_t len) VC_ATTR_NONNULL((1));

    
  typedef enum { vcFEATURE_VKILL,  vcFEATURE_IATTR,   vcFEATURE_RLIMIT,
		 vcFEATURE_COMPAT, vcFEATURE_MIGRATE, vcFEATURE_NAMESPACE,
		 vcFEATURE_SCHED,  vcFEATURE_VINFO,   vcFEATURE_VHI,
                 vcFEATURE_VSHELPER0, vcFEATURE_VSHELPER, vcFEATURE_VWAIT,
		 vcFEATURE_VNET, vcFEATURE_VSTAT,     vcFEATURE_PPTAG, }
    vcFeatureSet;

  bool		vc_isSupported(vcFeatureSet) VC_ATTR_CONST;
  bool		vc_isSupportedString(char const *);

  
  typedef enum { vcTYPE_INVALID, vcTYPE_MAIN, vcTYPE_WATCH,
		 vcTYPE_STATIC, vcTYPE_DYNAMIC }
    vcXidType;
  
  vcXidType	vc_getXIDType(xid_t xid) VC_ATTR_CONST;

    /** Returns true iff \a xid is a dynamic xid */
  bool		vc_is_dynamic_xid(xid_t xid);


  /* The management part */

#define VC_LIMIT_VSERVER_NAME_LEN	1024
  
  typedef enum { vcCFG_NONE, vcCFG_AUTO,
		 vcCFG_LEGACY,
		 vcCFG_RECENT_SHORT,
		 vcCFG_RECENT_FULL }		vcCfgStyle;


  /** Maps an xid given at '--xid' options to an xid_t */
  xid_t		vc_xidopt2xid(char const *, bool honor_static, char const **err_info);
  /** Maps a  nid given at '--nid' options to a  nid_t */
  nid_t		vc_nidopt2nid(char const *, bool honor_static, char const **err_info);
  /** Maps a  tag given at '--tag' options to a  tag_t */
  tag_t		vc_tagopt2tag(char const *, bool honor_static, char const **err_info);

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
  char *	vc_getVserverVdir(char const *id, vcCfgStyle style, bool physical);

  typedef enum { vcCTX_XID = 1,
		 vcCTX_NID,
		 vcCTX_TAG,
	} vcCtxType;

  /** Returns the ctx of the given vserver. When vserver is not running and
   *  'honor_static' is false, VC_NOCTX will be returned. Else, when
   *  'honor_static' is true and a static assignment exists, those value will
   *  be returned. Else, the result will be VC_NOCTX.
   *
   *  When 'is_running' is not null, the status of the vserver will be
   *  assigned to this variable. */
  xid_t		vc_getVserverCtx(char const *id, vcCfgStyle style,
				 bool honor_static, bool /*@null@*/ *is_running,
				 vcCtxType type);

  /** Resolves the cfg-path of the vserver owning the given ctx. 'revdir' will
      be used as the directory holding the mapping-links; when NULL, the
      default value will be assumed.  The result will be allocated and must be
      freed by the caller. */
  char *	vc_getVserverByCtx(xid_t ctx, /*@null@*/vcCfgStyle *style,
				   /*@null@*/char const *revdir);

  int		vc_compareVserverById(char const *lhs, vcCfgStyle lhs_style,
				      char const *rhs, vcCfgStyle rhs_style);
 
#define vcSKEL_INTERFACES	1u
#define vcSKEL_PKGMGMT		2u
#define vcSKEL_FILESYSTEM	4u

  /** Create a basic configuration skeleton for a vserver plus toplevel
   *  directories for pkgmanagemt and filesystem (when requested). */
  int		vc_createSkeleton(char const *id, vcCfgStyle style, int flags);


#ifdef __cplusplus
}
#endif

#undef VC_ATTR_PURE
#undef VC_ATTR_ALWAYSINLINE
#undef VC_ATTR_NORETURN
#undef VC_ATTR_UNUSED
#undef VC_ATTR_NONNULL

#endif
