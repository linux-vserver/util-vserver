// $Id$    --*- c -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "lib/utils-legacy.h"
#include "pathconfig.h"
#include "util.h"

#include "internal.h"
#include "vserver.h"

#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <strings.h>

#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_IO	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#undef _POSIX_SOURCE
#include "capability-compat.h"

typedef enum { tgNONE,tgCONTEXT, tgID, tgRUNNING,
	       tgVDIR, tgNAME, tgCFGDIR, tgAPPDIR,
	       tgAPIVER, tgPXID,
	       tgINITPID, tgINITPID_PID,
	       tgXID, tgUTS, tgSYSINFO,
	       tgFEATURE, tgCANONIFY,
	       tgVERIFYCAP, tgXIDTYPE, tgVERIFYPROC,
	       tgNID, tgTAG,
}	VserverTag;

static struct {
    char const * const	tag;
    VserverTag const	val;
    char const * const	descr;
}  const TAGS[] = {
  { "CONTEXT", tgCONTEXT, ("the current and/or assigned context; when an optinal argument "
			   "evaluates to false,only the current context will be printed") },
  { "ID",      tgID,      "gives out the vserver-id for the context-xid" },
  { "RUNNING", tgRUNNING, "gives out '1' when vserver is running; else, it fails without output" },
  { "VDIR",    tgVDIR,    "gives out the root-directory of the vserver" },
  { "NAME",    tgNAME,    "gives out the name of the vserver" },
  { "CFGDIR",  tgCFGDIR,  "gives out the configuration directory of the vserver" },
  { "APPDIR",  tgAPPDIR,  "gives out the name of the toplevel application cfgdir" },
  { "INITPID",     tgINITPID,     "gives out the initpid of the given context" },
  { "INITPID_PID", tgINITPID_PID, "gives out the initpid of the given pid" },
  { "XID",         tgXID,         "gives out the context-id of the given pid" },
  { "APIVER",      tgAPIVER,      "gives out the version of the kernel API" },
  { "UTS",         tgUTS,         ("gives out an uts-entry; possible entries are "
				   "context, sysname, nodename, release, version, "
				   "machine and domainname") },
  { "SYSINFO",     tgSYSINFO,     "gives out information about the systen" },
  { "FEATURE",     tgFEATURE,     "returns 0 iff the queried feature is supported" },
  { "PXID",        tgPXID,        "returns the xid of the parent context" },
  { "CANONIFY",    tgCANONIFY,    "canonifies the vserver-name and removes dangerous characters" },
  { "VERIFYCAP",   tgVERIFYCAP,   "test if the kernel supports linux capabilities" },
  { "VERIFYPROC",  tgVERIFYPROC,  "test if /proc can be read by contexts!=0" },
  { "XIDTYPE",     tgXIDTYPE,     "returns the type of the given XID" },
  { "NID",         tgNID,         "outputs the network context-id of the given pid" },
  { "TAG",         tgTAG,         "outputs the filesystem tag of the given pid" },
};

int wrapper_exit_code = 1;

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, 'h' },
  { "version",  no_argument,  0, 'v' },
  { 0,0,0,0 }
};


static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-ql] <vserver>|<pid>|<context> <tag>\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vserver-info " VERSION " -- returns information about vservers\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
showTags()
{
  char const *		delim = "";
  size_t	i;

  WRITE_MSG(1, "Valid tags are: ");
  for (i=0; i<DIM_OF(TAGS); ++i) {
    WRITE_STR(1, delim);
    WRITE_STR(1, TAGS[i].tag);

    delim = ", ";
  }
  WRITE_MSG(1, "\n");
  exit(0);
}

static VserverTag
stringToTag(char const *str)
{
  size_t	i;
  for (i=0; i<DIM_OF(TAGS); ++i)
    if (strcmp(TAGS[i].tag, str)==0) return TAGS[i].val;

  return tgNONE;
}

static vc_uts_type
utsText2Tag(char const *str)
{
  if      (strcmp(str, "context")   ==0) return vcVHI_CONTEXT;
  else if (strcmp(str, "sysname")   ==0) return vcVHI_SYSNAME;
  else if (strcmp(str, "nodename")  ==0) return vcVHI_NODENAME;
  else if (strcmp(str, "release")   ==0) return vcVHI_RELEASE;
  else if (strcmp(str, "version")   ==0) return vcVHI_VERSION;
  else if (strcmp(str, "machine")   ==0) return vcVHI_MACHINE;
  else if (strcmp(str, "domainname")==0) return vcVHI_DOMAINNAME;
  else {
    WRITE_MSG(2, "Unknown UTS tag\n");
    exit(1);
  }
}

static bool
verifyProc()
{
  char const		*errptr;
  
  if (!switchToWatchXid(&errptr)) {
    perror(errptr);
    return false;
  }

  if (access("/proc/uptime", R_OK)==-1) {
    if (errno!=ENOENT)
      perror("access(\"/proc/uptime\")");
    
    return false;
  }

  return true;
}

static bool
verifyCap()
{
  int retried = 0;
  struct __user_cap_header_struct header;
  struct __user_cap_data_struct user[2];

  header.version = _LINUX_CAPABILITY_VERSION_3;
  header.pid     = 0;

  if (getuid()!=0) {
    WRITE_MSG(2, "'VERIFYCAP' can be executed as root only\n");
    return false;
  }

//  if( prctl( PR_SET_KEEPCAPS, 1,0,0,0 ) < 0 ) {
//    perror( "prctl:" );
//    return false;
//  }

retry:
  if (capget(&header, user)==-1) {
    if (!retried &&
	header.version != _LINUX_CAPABILITY_VERSION_3) {
      header.version = _LINUX_CAPABILITY_VERSION_1;
      retried = 1;
      goto retry;
    }
    perror("capget()");
    return false;
  }

  user[0].effective   = user[1].effective   = 0;
  user[0].permitted   = user[1].permitted   = 0;
  user[0].inheritable = user[1].inheritable = 0;

  if (capset(&header, user)==-1) {
    perror("capset()");
    return false;
  }

  return chroot("/")==-1;
}

static char *
getAPIVer(char *buf)
{
  int		v = vc_get_version();
  size_t	l;
  
  if (v==-1) return 0;

  
  l = utilvserver_fmt_xulong(0, (unsigned int)v);
  memcpy(buf, "0x00000000", 10);
  utilvserver_fmt_xulong(buf+2+8-l, (unsigned int)v);

  return buf;
}

static inline char *
getCtxId(char *buf, const char *pid_str, xid_t (*get_id)(pid_t pid), const char *err_str)
{
  pid_t		pid = atoi(pid_str);
  xid_t		xid = get_id(pid);

  if (xid==VC_NOCTX) perror(err_str);
  else {
    utilvserver_fmt_long(buf, xid);
    return buf;
  }

  return 0;
}

static char *
getXid(char *buf, char const *pid_str)
{
  return getCtxId(buf, pid_str, vc_get_task_xid, "vc_get_task_xid()");
}

static char *
getNid(char *buf, const char *pid_str)
{
  return getCtxId(buf, pid_str, vc_get_task_nid, "vc_get_task_nid()");
}

static char *
getTag(char *buf, const char *pid_str)
{
  return getCtxId(buf, pid_str, vc_get_task_tag, "vc_get_task_tag()");
}

static char *
getPXid(char UNUSED *buf, char const UNUSED *vserver)
{
  // TODO: implement me when available
  return 0;
}

static char *
getInitPid_native(char *buf, xid_t xid)
{
  struct vc_vx_info		info;
  
  if (vc_get_vx_info(xid, &info)==-1) perror("vc_get_vx_info()");
  else {
    utilvserver_fmt_long(buf, info.initpid);
    return buf;
  }

  return 0;
}

#if defined(VC_ENABLE_API_COMPAT) || defined(VC_ENABLE_API_V11)
static int
selectPid(struct dirent const *ent)
{
  return atoi(ent->d_name)!=0;
}

static bool
getInitPid_internal(pid_t pid, xid_t xid, pid_t *res)
{
  *res = -1;
  
  for (;*res==-1;) {
    size_t			bufsize = utilvserver_getProcEntryBufsize();
    char			buf[bufsize+1];
    char			*pos = 0;

    pos = utilvserver_getProcEntry(pid, "\ns_context: ", buf, bufsize);
    if (pos==0 && errno==EAGAIN) continue;

    if (pos==0 || (xid_t)atoi(pos)!=xid) return false;

    buf[bufsize] = '\0';
    pos          = strstr(buf, "\ninitpid: ");
    
    if (pos!=0) {
      pos       += sizeof("\ninitpid: ")-1;
      if (strncmp(pos, "none", 4)==0) *res = -1;
      else                            *res = atoi(pos);
    }
  }

  return true;
}

static char *
getInitPid_emulated(char *buf, xid_t xid)
{
  struct dirent **namelist;
  int		n;

  switchToWatchXid(0);	// ignore errors silently...
  n = scandir("/proc", &namelist, selectPid, alphasort);
  if (n<0) perror("scandir()");
  else while (n--) {
    pid_t	pid;
    if (!getInitPid_internal(atoi(namelist[n]->d_name), xid, &pid)) continue;

    utilvserver_fmt_long(buf, pid);
    return buf;
  }

  return 0;
}
#else // VC_ENABLE_API_COMPAT
static char *
getInitPid_emulated(char UNUSED *buf, xid_t UNUSED xid)
{
  WRITE_MSG(2, "tools were built without compat API, getInitPid() not available\n");
  return 0;
}
#endif // VC_ENABLE_API_COMPAT

static char *
getInitPid(char *buf, xid_t xid)
{
  if (vc_isSupported(vcFEATURE_VINFO))
    return getInitPid_native(buf, xid);
  else
    return getInitPid_emulated(buf, xid);
}

static char *
getInitPidPid(char *buf, char const *vserver)
{
  struct vc_vx_info	info;
  pid_t			pid = atoi(vserver);
  xid_t 		xid = vc_get_task_xid(pid);

  if (xid==VC_NOCTX) perror("vc_get_task_xid()");
  else if (vc_get_vx_info(xid, &info)==-1) perror("vc_get_vx_info()");
  else {
    utilvserver_fmt_long(buf, info.initpid);
    return buf;
  }

  return 0;
}

static char *
getUTS(char *buf, xid_t xid, size_t argc, char * argv[])
{
  if (argc>0) {
    vc_uts_type	type = utsText2Tag(argv[0]);
    if (vc_get_vhi_name(xid, type, buf, sizeof(buf)-1)==-1)
      perror("vc_get_vhi_name()");
    else
      return buf;
  }
  else {
    bool		is_passed = false;
    char		tmp[128];
#define APPEND_UTS(TYPE)						\
    (((vc_get_vhi_name(xid, TYPE, tmp, sizeof(tmp)-1)!=-1) && (strcat(buf, tmp), strcat(buf, " "), is_passed=true)) || \
     (strcat(buf, "??? ")))

    if (APPEND_UTS(vcVHI_CONTEXT) &&
	APPEND_UTS(vcVHI_SYSNAME) &&
	APPEND_UTS(vcVHI_NODENAME) &&
	APPEND_UTS(vcVHI_RELEASE) &&
	APPEND_UTS(vcVHI_VERSION) &&
	APPEND_UTS(vcVHI_MACHINE) &&
	APPEND_UTS(vcVHI_DOMAINNAME) &&
	is_passed)
      return buf;

    perror("vc_get_vhi_name()");
#undef APPEND_UTS
  }

  return 0;
}

static int
printSysInfo(char *buf)
{
  int			fd = open(PKGLIBDIR "/FEATURES.txt", O_RDONLY);
  struct utsname	uts;

  if (uname(&uts)==-1)
    perror("uname()");
  else {
    WRITE_MSG(1,
	      "Versions:\n"
	      "                   Kernel: ");
    WRITE_STR(1, uts.release);
    WRITE_MSG(1, "\n"
	      "                   VS-API: ");

    memset(buf, 0, 128);
    if (getAPIVer(buf)) WRITE_STR(1, buf);
    else                WRITE_MSG(1, "???");
    
    WRITE_MSG(1, "\n"
	      "             util-vserver: " PACKAGE_VERSION "; " __DATE__ ", " __TIME__"\n"
	      "\n");
  }

  if (fd==-1)
    WRITE_MSG(1, "FEATURES.txt not found\n");
  else {
    off_t		l  = Elseek(fd, 0, SEEK_END);
    Elseek(fd, 0, SEEK_SET);
    {
      char		buf[l];
      EreadAll(fd, buf, l);
      EwriteAll(1, buf, l);
    }
    Eclose(fd);
  }

  return EXIT_SUCCESS;
}

static char *
getContext(char *buf, char const *vserver, bool allow_only_static)
{
  xid_t		xid = vc_getVserverCtx(vserver, vcCFG_AUTO,
				       allow_only_static, 0, vcCTX_XID);
  if (xid==VC_NOCTX) return 0;
  
  utilvserver_fmt_long(buf, xid);
  return buf;
}

static char const *
getXIDType(xid_t xid, int argc, char *argv[])
{
  char const *		tp;
  
  switch (vc_getXIDType(xid)) {
    case vcTYPE_INVALID		:  tp = "invalid"; break;
    case vcTYPE_MAIN		:  tp = "main";    break;
    case vcTYPE_WATCH		:  tp = "watch";   break;
    case vcTYPE_STATIC		:  tp = "static";  break;
    case vcTYPE_DYNAMIC		:  tp = "dynamic"; break;
    default			:  tp = 0;         break;
  }

  if (argc==0 || tp==0)
    return tp;

  while (argc>0) {
    --argc;
    if (strcasecmp(argv[argc], tp)==0) return tp;
  }

  return 0;
}

static int
testFeature(int argc, char *argv[])
{
  return (argc>0 && vc_isSupportedString(argv[0])) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static bool
str2bool(char const *str)
{
  return atoi(str)!=0 || strchr("yYtT", str[0])!=0 || strcasecmp("true", str)==0;
}

static int
execQuery(char const *vserver, VserverTag tag, int argc, char *argv[])
{
  char const *		res = 0;
  char 			buf[sizeof(xid_t)*4 + 1024 + strlen(vserver)];

  memset(buf, 0, sizeof buf);
  switch (tag) {
    case tgNAME		:  res = vc_getVserverName(vserver, vcCFG_AUTO); break;
    case tgVDIR		:
      res = vc_getVserverVdir(vserver, vcCFG_AUTO, argc>0 && atoi(argv[0]));
      break;
    case tgCFGDIR	:  res = vc_getVserverCfgDir(vserver, vcCFG_AUTO);     break;
    case tgAPPDIR	:
      res = vc_getVserverAppDir(vserver, vcCFG_AUTO, argc==0 ? "" : argv[0]);
      break;
      
    case tgRUNNING	: {
      signed long		xid;	// type is a small hack, but should be ok...
      struct vc_vx_info		info;
	
      if (isNumber(vserver, &xid, true) && xid>=0)
	res = (vc_get_vx_info(xid, &info)==-1) ? 0 : "1";
      else
	res = (vc_getVserverCtx(vserver, vcCFG_AUTO, false, 0, vcCTX_XID)==VC_NOCTX) ? 0 : "1";
      
      break;
    }

    case tgCANONIFY	:
      strcpy(buf, vserver);
      if (canonifyVserverName(buf)>0) res = buf;
      break;
      
    case tgCONTEXT	:  res = getContext(buf, vserver,
					    argc==0 || str2bool(argv[0])); break;
    case tgINITPID_PID	:  res = getInitPidPid(buf, vserver);  break;
    case tgAPIVER	:  res = getAPIVer(buf);               break;
    case tgXID		:  res = getXid(buf, vserver);         break;
    case tgPXID		:  res = getPXid(buf, vserver);        break;
    case tgSYSINFO	:  return printSysInfo(buf);           break;
    case tgFEATURE	:  return testFeature(argc,argv);      break;
    case tgVERIFYCAP	:  return verifyCap() ? 0 : 1;         break;
    case tgVERIFYPROC	:  return verifyProc() ? 0 : 1;        break;
    case tgNID		:  res = getNid(buf, vserver);         break;
    case tgTAG		:  res = getTag(buf, vserver);         break;


    default		: {
      xid_t		xid = *vserver!='\0' ? vc_xidopt2xid(vserver,true,0) : VC_SAMECTX;

      switch (tag) {
	case tgID	:  res = vc_getVserverByCtx(xid,0,0);  break;
	case tgINITPID	:  res = getInitPid(buf, xid);         break;
	case tgUTS	:  res = getUTS(buf, xid, argc, argv); break;
	case tgXIDTYPE	:  res = getXIDType(xid, argc, argv);  break;
    
	default		:  assert(false); abort();  // TODO
      }
    }
  }

  if (res==0) return EXIT_FAILURE;
  WRITE_STR(1, res);
  WRITE_MSG(1, "\n");
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  bool		quiet = false;
  char const *	vserver;
  VserverTag	tag;
  
  while (1) {
    int		c = getopt_long(argc, argv, "ql", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case 'v'		:  showVersion();
      case 'l'		:  showTags();
      case 'q'		:  quiet = true; break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	exit(1);
	break;
    }
  }

  if (optind+2>argc) {
    execQuery("-", tgSYSINFO, 0, 0);
    WRITE_MSG(2, "\nAssumed 'SYSINFO' as no other option given; try '--help' for more information.\n");
    exit(0);
  }

  vserver = argv[optind];
  tag     = stringToTag(argv[optind+1]);

  if (tag==tgNONE) {
    WRITE_MSG(2, "Unknown tag; use '-l' to get list of valid tags\n");
    exit(1);
  }

  if (quiet) {
    int		fd = Eopen("/dev/null", O_WRONLY, 0644);
    Edup2(fd, 1);
    Eclose(fd);
  }

  return execQuery(vserver, tag, argc-(optind+2), argv+optind+2);
}
