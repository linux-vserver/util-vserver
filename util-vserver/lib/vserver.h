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

#ifndef IS_DOXYGEN
#if defined(__GNUC__)
#  define VC_ATTR_UNUSED                __attribute__((__unused__))
#  define VC_ATTR_NORETURN              __attribute__((__noreturn__))
#  define VC_ATTR_CONST			__attribute__((__const__))
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
#  define VC_ATTR_PURE
#  define VC_ATTR_CONST
#endif
#endif	// IS_DOXYGEN

/** the value which is returned in error-case (no ctx found) */
#define VC_NOCTX		((xid_t)(-1))
/** the value which means a random (the next free) ctx */
#define VC_DYNAMIC_XID		((xid_t)(-1))
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
#define VC_CAP_QUOTACTL          	29

#define VC_IMMUTABLE_FILE_FL		0x00000010l
#define VC_IMMUTABLE_LINK_FL		0x00008000l
#define VC_IMMUTABLE_ALL		(VC_IMMUTABLE_LINK_FL|VC_IMMUTABLE_FILE_FL)

#define VC_IATTR_XID			0x01000000

#define VC_IATTR_ADMIN			0x00000001
#define VC_IATTR_WATCH			0x00000002
#define VC_IATTR_HIDE			0x00000004
#define VC_IATTR_FLAGS			0x00000007

#define VC_IATTR_BARRIER		0x00010000
#define	VC_IATTR_IUNLINK		0x00020000
#define VC_IATTR_IMMUTABLE		0x00040000


// the flags
#define VC_VXF_INFO_LOCK		0x00000001
#define VC_VXF_INFO_NPROC		0x00000002
#define VC_VXF_INFO_PRIVATE		0x00000004
#define VC_VXF_INFO_INIT		0x00000008

#define VC_VXF_INFO_HIDE		0x00000010
#define VC_VXF_INFO_ULIMIT		0x00000020
#define VC_VXF_INFO_NSPACE		0x00000040

#define	VC_VXF_SCHED_HARD		0x00000100
#define	VC_VXF_SCHED_PRIO		0x00000200
#define	VC_VXF_SCHED_PAUSE		0x00000400

#define VC_VXF_VIRT_MEM			0x00010000
#define VC_VXF_VIRT_UPTIME		0x00020000

#define	VC_VXF_STATE_SETUP		(1ULL<<32)
#define	VC_VXF_STATE_INIT		(1ULL<<33)


/** \defgroup  syscalls Syscall wrappers
 *  Functions which are calling the vserver syscall directly. */

/** \defgroup  helper   Helper functions
 *  Functions which are doing general helper tasks like parameter parsing. */

/** \typedef  an_unsigned_integer_type  xid_t
 *  The identifier of a context. */

#ifdef IS_DOXYGEN
typedef an_unsigned_integer_type	xid_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

  struct vc_ip_mask_pair {
    uint32_t	ip;
      uint32_t	mask;
  };

    /** \brief   Returns the version of the current kernel API.
     *  \ingroup syscalls
     *	\returns The versionnumber of the kernel API
     */
  int	vc_get_version();
  
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
     *  \returns  The new context-id, or VC_NOCTX on errors; errno
     *	          will be set appropriately
     *
     *  See http://vserver.13thfloor.at/Stuff/Logic.txt for details */
  xid_t	vc_new_s_context(xid_t ctx, unsigned int remove_cap, unsigned int flags);

    /** \brief  Sets the ipv4root information.
     *  \ingroup syscalls
     *  \pre    \a nb < NB_IPV4ROOT && \a ips != 0 */
  int	vc_set_ipv4root(uint32_t  bcast, size_t nb,
			struct vc_ip_mask_pair const *ips) VC_ATTR_NONNULL((3));

    /** \brief  Returns the value of NB_IPV4ROOT.
     *  \ingroup helper
     *
     *  This function returns the value of NB_IPV4ROOT which was used when the
     *  library was built, but \b not the value which is used by the currently
     *  running kernel. */
  size_t	vc_get_nb_ipv4root() VC_ATTR_CONST VC_ATTR_PURE;

    /** \brief   Creates a context without starting it.
     *  \ingroup syscalls
     *
     *  This functions initializes a new context. When already in a freshly
     *  created context, this old context will be discarded.
     *
     *  \param xid  The new context; special values are:
     *	- VC_DYNAMIC_XID which means to create a dynamic context
     *
     *	\returns the xid of the created context, or VC_NOCTX on errors. errno
     *	         will be set appropriately. */
  xid_t	vc_create_context(xid_t xid);

    /** \brief   Moves the current process into the specified context.
     *  \ingroup syscalls
     *
     *  \param   xid  The new context
     *  \returns 0 on success, -1 on errors */
  int	vc_migrate_context(xid_t xid);
  
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

    /** \brief  The limits of a resources.
     *
     *  This is a triple consisting of a minimum, soft and hardlimit. */
  struct vc_rlimit {
      vc_limit_t	min;	///< the guaranted minimum of a resources
      vc_limit_t	soft;	///< the softlimit of a resource
      vc_limit_t	hard;	///< the absolute hardlimit of a resource
  };

    /** \brief  Masks describing the supported limits. */
  struct  vc_rlimit_mask {
      uint_least32_t	min;	///< masks the resources supporting a minimum limit
      uint_least32_t	soft;	///< masks the resources supporting a soft limit
      uint_least32_t	hard;	///< masks the resources supporting a hard limit
  };

    /** \brief   Returns the limits of \a resource.
     *  \ingroup syscalls
     *
     *  \param  xid       The id of the context
     *  \param  resource  The resource which will be queried
     *  \param  lim       The result which will be filled with the limits
     *
     *  \returns 0 on success, and -1 on errors. */
  int	vc_get_rlimit(xid_t xid, int resource,
		      struct vc_rlimit       /*@out@*/ *lim) VC_ATTR_NONNULL((3));
    /** \brief   Sets the limits of \a resource.
     *  \ingroup syscalls
     *
     *  \param  xid       The id of the context
     *  \param  resource  The resource which will be queried
     *  \param  lim       The new limits
     *
     *  \returns 0 on success, and -1 on errors. */
  int	vc_set_rlimit(xid_t xid, int resource,
		      struct vc_rlimit const /*@in@*/  *lim) VC_ATTR_NONNULL((3));
  int	vc_get_rlimit_mask(xid_t xid,
			   struct vc_rlimit_mask *lim)       VC_ATTR_NONNULL((2));
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
  bool	vc_parseLimit(char const /*@in@*/ *str, vc_limit_t /*@out@*/ *res)	VC_ATTR_NONNULL((1,2));


  /** \brief    Sends a signal to a context/pid
   *  \ingroup  syscalls
   *
   *  Special values for \a pid are:
   *  - -1   which means every process in ctx except the init-process
   *  -  0   which means every process in ctx inclusive the init-process */
  int	vc_ctx_kill(xid_t ctx, pid_t pid, int sig);



  int		vc_set_iattr(char const *filename, xid_t xid,
			     uint_least32_t flags, uint_least32_t mask) VC_ATTR_NONNULL((1));
  int		vc_get_iattr(char const *filename, xid_t * /*@null@*/ xid,
			     uint_least32_t * /*@null@*/ flags,
			     uint_least32_t * /*@null@*/ mask) VC_ATTR_NONNULL((1));

  struct vc_vx_info {
      xid_t	xid;
      pid_t	initpid;
  };
  
    /** Returns the context of the given process. pid==0 means the current process. */
  xid_t		vc_get_task_xid(pid_t pid);
  int		vc_get_vx_info(xid_t xid, struct vc_vx_info *info) VC_ATTR_NONNULL((2));


  typedef enum { vcVHI_CONTEXT, vcVHI_SYSNAME, vcVHI_NODENAME,
		 vcVHI_RELEASE, vcVHI_VERSION, vcVHI_MACHINE,
		 vcVHI_DOMAINNAME }		vc_uts_type;
  
  int		vc_set_vhi_name(xid_t xid, vc_uts_type type,
				char const *val, size_t len) VC_ATTR_NONNULL((3));
  int		vc_get_vhi_name(xid_t xid, vc_uts_type type,
				char *val, size_t len)       VC_ATTR_NONNULL((3));

    /** Returns true iff \a xid is a dynamic xid */
  bool		vc_is_dynamic_xid(xid_t xid);

  int		vc_enter_namespace(xid_t xid);
  int		vc_set_namespace();
  int		vc_cleanup_namespace();

  struct  vc_ctx_flags {
      uint_least64_t	flagword;
      uint_least64_t	mask;
  };
  
  struct  vc_ctx_caps {
      uint_least64_t	bcaps;
      uint_least64_t	bmask;
      uint_least64_t	ccaps;
      uint_least64_t	cmask;
  };

  struct vc_err_listparser {
      char const	*ptr;
      size_t		len;
  };
 
  int			vc_get_flags(xid_t xid, struct vc_ctx_flags *)       VC_ATTR_NONNULL((2));
  int			vc_set_flags(xid_t xid, struct vc_ctx_flags const *) VC_ATTR_NONNULL((2));

  int			vc_get_ccaps(xid_t xid, struct vc_ctx_caps *);
  int			vc_set_ccaps(xid_t xid, struct vc_ctx_caps const *);

  uint_least64_t	vc_text2bcap(char const *, size_t len);
  char const *		vc_lobcap2text(uint_least64_t *);
  int			vc_list2bcap(char const *, size_t len,
				     struct vc_err_listparser *err,
				     struct vc_ctx_caps *);

  uint_least64_t	vc_text2ccap(char const *, size_t len);
  char const *		vc_loccap2text(uint_least64_t *);
  int			vc_list2ccap(char const *, size_t len,
				     struct vc_err_listparser *err,
				     struct vc_ctx_caps *);

  int			vc_list2flag(char const *, size_t len,
				     struct vc_err_listparser *err,
				     struct vc_ctx_flags *flags);
  uint_least64_t	vc_text2flag(char const *, size_t len);
  char const *		vc_loflag2text(uint_least64_t *);
  
  uint_least32_t	vc_list2flag_compat(char const *, size_t len,
					    struct vc_err_listparser *err);
  uint_least32_t	vc_text2flag_compat(char const *, size_t len);
  char const *		vc_hiflag2text_compat(uint_least32_t);

  uint_least32_t	vc_get_insecurecaps() VC_ATTR_CONST;
  int			vc_text2cap(char const *);
  char const *		vc_cap2text(unsigned int);


  inline static int	vc_setfilecontext(char const *filename, xid_t xid) {
    return vc_set_iattr(filename, xid, 0, VC_IATTR_XID);
  }
  
  inline static xid_t	vc_getfilecontext(char const *filename) {
    xid_t	res;
    if (vc_get_iattr(filename, &res, 0,0)==-1) return VC_NOCTX;
    return res;
  }


  struct vc_set_sched {
      int32_t	fill_rate;
      int32_t	interval;
      int32_t	tokens;
      int32_t	tokens_min;
      int32_t	tokens_max;
      uint64_t	cpu_mask;
  };

  int		vc_set_sched(xid_t xid, struct vc_set_sched const *);
  
  
  typedef enum { vcFEATURE_VKILL,  vcFEATURE_IATTR,   vcFEATURE_RLIMIT,
		 vcFEATURE_COMPAT, vcFEATURE_MIGRATE, vcFEATURE_NAMESPACE,
		 vcFEATURE_SCHED,  vcFEATURE_VINFO,   vcFEATURE_VHI,
                 vcFEATURE_VSHELPER }
    vcFeatureSet;

  bool		vc_isSupported(vcFeatureSet) VC_ATTR_CONST;
  bool		vc_isSupportedString(char const *);

  /* The management part */

#define VC_LIMIT_VSERVER_NAME_LEN	1024
  
  typedef enum { vcCFG_NONE, vcCFG_AUTO,
		 vcCFG_LEGACY,
		 vcCFG_RECENT_SHORT,
		 vcCFG_RECENT_FULL }		vcCfgStyle;


  /** Maps an xid given at '--xid' options to an xid_t */
  xid_t		vc_xidopt2xid(char const *, bool honor_static, char const **err_info);

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
