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

#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_IO	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

typedef enum { tgNONE,tgCONTEXT, tgRUNNING,
	       tgVDIR, tgNAME, tgCFGDIR, tgAPPDIR,
	       tgAPIVER,
	       tgINITPID, tgINITPID_PID,
	       tgXID, tgUTS, tgSYSINFO,
}	VserverTag;

static struct {
    char const * const	tag;
    VserverTag const	val;
    char const * const	descr;
}  const TAGS[] = {
  { "CONTEXT", tgCONTEXT, "the current and/or assigned context" },
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

static char *
getXid(char *buf, char const *vserver)
{
  pid_t		pid = atoi(vserver);
  xid_t		xid = vc_get_task_xid(pid);

  if (xid==VC_NOCTX) perror("vc_get_task_xid()");
  else {
    utilvserver_fmt_long(buf, xid);
    return buf;
  }

  return 0;
}

static char *
getInitPid(char *buf, xid_t xid)
{
  struct vc_vx_info		info;
  
  if (vc_get_vx_info(xid, &info)==-1) perror("vc_get_vx_info()");
  else {
    utilvserver_fmt_long(buf, info.xid);
    return buf;
  }

  return 0;
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
    utilvserver_fmt_long(buf, info.xid);
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
getContext(char *buf, char const *vserver)
{
  xid_t		xid = vc_getVserverCtx(vserver, vcCFG_AUTO, true, 0);
  if (xid==VC_NOCTX) return 0;
  
  utilvserver_fmt_long(buf, xid);
  return buf;
}

static int
execQuery(char const *vserver, VserverTag tag, int argc, char *argv[])
{
  char const *		res = 0;
  char 			buf[sizeof(xid_t)*4 + 1024];
  xid_t			xid = *vserver!='\0' ? (xid_t)(atoi(vserver)) : VC_SAMECTX;

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
      
    case tgRUNNING	:
      res = (vc_getVserverCtx(vserver, vcCFG_AUTO, false, 0)==VC_NOCTX) ? 0 : "1";
      break;

    case tgCONTEXT	:  res = getContext(buf, vserver);     break;
    case tgXID		:  res = getXid(buf, vserver);         break;
    case tgINITPID	:  res = getInitPid(buf, xid);         break;
    case tgINITPID_PID	:  res = getInitPidPid(buf, vserver);  break;
    case tgAPIVER	:  res = getAPIVer(buf);               break;
    case tgUTS		:  res = getUTS(buf, xid, argc, argv); break;
    case tgSYSINFO	:  return printSysInfo(buf);           break;
    
    default		:  assert(false); abort();  // TODO
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
	WRITE_MSG(2, " --help\" for more information.\n");
	exit(1);
	break;
    }
  }

  if (optind+2>argc) {
    WRITE_MSG(2, "No vserver or tag give; please try '--help' for more information.\n");
    exit(1);
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
