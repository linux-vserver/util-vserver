// $Id$

// Copyright (C) 2003,2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on chcontext.cc by Jacques Gelinas
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
	chcontext is a wrapper to user the new_s_context system call. It
	does little more than mapping command line option to the system call
	arguments.
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util.h"
#include "vserver.h"
#include "internal.h"
#include "lib_internal/jail.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>
#include <fcntl.h>
#include <libgen.h>

#define ENSC_WRAPPERS_PREFIX	"chcontext: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#include <wrappers.h>

#define CMD_HELP	0x1000
#define CMD_VERSION	0x1001
#define CMD_CAP		0x4000
#define CMD_CTX		0x4001
#define CMD_DISCONNECT	0x4002
#define CMD_DOMAINNAME	0x4003
#define CMD_FLAG	0x4004
#define CMD_HOSTNAME	0x4005
#define CMD_SECURE	0x4006
#define CMD_SILENT	0x4007

int wrapper_exit_code	= 255;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "cap",        required_argument,  0, CMD_CAP },
  { "ctx",        required_argument,  0, CMD_CTX },
  { "xid",        required_argument,  0, CMD_CTX },
  { "disconnect", no_argument,        0, CMD_DISCONNECT },
  { "domainname", required_argument,  0, CMD_DOMAINNAME },
  { "flag",       required_argument,  0, CMD_FLAG },
  { "hostname",   required_argument,  0, CMD_HOSTNAME },
  { "secure",     no_argument,        0, CMD_SECURE },
  { "silent",     no_argument,        0, CMD_SILENT },
  { 0,0,0,0 }
};

struct Arguments {
    size_t		nbctx;
    xid_t		ctxs[16];
    bool		do_disconnect;
    bool		do_silent;
    unsigned int	flags;
    uint32_t		remove_caps;
    uint32_t		add_caps;
    char const *	hostname;
    char const *	domainname;
};

static struct Arguments const *		global_args = 0;

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd, "Usage: ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--cap [!]<cap_name>] [--secure] [--xid <num>] [--disconnect]\n"
	    "       [--domainname <name>] [--hostname <name>] [--flag <flags>+]\n"
	    "       [--silent] [--] command arguments ...\n"
	    "\n"
	    "chcontext allocate a new security context and executes\n"
	    "a command in that context.\n"
	    "By default, a new/unused context is allocated\n"
	    "\n"
	    "--cap CAP_NAME\n"
	    "    Add a capability from the command. This option may be\n"
	    "    repeated several time.\n"
	    "    See /usr/include/linux/capability.h\n"
	    "    In general, this option is used with the --secure option\n"
	    "    --secure removes most critical capabilities and --cap\n"
	    "    adds specific ones.\n"
	    "\n"

	    "--cap !CAP_NAME\n"
	    "    Remove a capability from the command. This option may be\n"
	    "    repeated several time.\n"
	    "    See /usr/include/linux/capability.h\n"
	    "\n"
	    "--xid num\n"
	    "    Select the context. On root in context 0 is allowed to\n"
	    "    select a specific context.\n"
	    "    Context number 1 is special. It can see all processes\n"
	    "    in any contexts, but can't kill them though.\n"
	    "    Option --xid may be repeated several times to specify up to 16 contexts.\n"

	    "--disconnect\n"
	    "    Start the command in background and make the process\n"
	    "    a child of process 1.\n"

	    "--domainname new_domainname\n"
	    "    Set the domainname (NIS) in the new security context.\n"
	    "    Use \"none\" to unset the domain name.\n"

	    "--flag\n"
	    "    Set one flag in the new or current security context. The following\n"
	    "    flags are supported. The option may be used several time.\n"
	    "\n"
	    "        fakeinit: The new process will believe it is process number 1.\n"
	    "                  Useful to run a real /sbin/init in a vserver.\n"
	    "        lock:     The new process is trapped and can't use chcontext anymore.\n"
	    "        sched:    The new process and its children will share a common \n"
	    "                  execution priority.\n"
	    "        nproc:    Limit the number of process in the vserver according to\n"
	    "                  ulimit setting. Normally, ulimit is a per user thing.\n"
	    "                  With this flag, it becomes a per vserver thing.\n"
	    "        private:  No one can join this security context once created.\n"
	    "        ulimit:   Apply the current ulimit to the whole context\n"

	    "--hostname new_hostname\n"
	    "    Set the hostname in the new security context\n"
	    "    This is need because if you create a less privileged\n"
	    "    security context, it may be unable to change its hostname\n"

	    "--secure\n"
	    "    Remove all the capabilities to make a virtual server trustable\n"

	    "--silent\n"
	    "    Do not print the allocated context number.\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "chcontext-compat " VERSION " -- allocates/enters a security context\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static inline void
setCap(char const *str, uint32_t *add_caps, uint32_t *remove_caps)
{
  uint32_t	*cap;
  int		bit;
  
  if (str[0] != '!')
    cap = add_caps;
  else {
    cap = remove_caps;
    str++;
  }
	
  bit = vc_text2cap(str);
	
  if (bit!=-1) *cap |= (1<<bit);
  else {
    WRITE_MSG(2, "Unknown capability '");
    WRITE_STR(2, str);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }
}

static inline void
setFlags(char const *str, uint32_t *flags)
{
  struct vc_err_listparser	err;
  
  *flags = vc_list2cflag_compat(str, 0, &err);

  if (err.ptr!=0) {
    WRITE_MSG(2, "Unknown flag '");
    write(2, err.ptr, err.len);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }
}

static inline ALWAYSINLINE void
setHostname(char const *name)
{
  if (name == NULL) return;
  
  if (sethostname(name, strlen(name))==-1) {
    perror("chcontext: sethostname()");
    exit(255);
  }
  if (!global_args->do_silent) {
    WRITE_MSG(1, "Host name is now ");
    WRITE_STR(1, name);
    WRITE_MSG(1, "\n");
  }
}

static inline ALWAYSINLINE void
setDomainname(char const *name)
{
  if (name == NULL) return;
  
  if (setdomainname(name, strlen(name))==-1) {
    perror("chcontext: setdomainname()");
    exit(255);
  }
  if (!global_args->do_silent) {
    WRITE_MSG(1, "Domain name is now ");
    WRITE_STR(1, name);
    WRITE_MSG(1, "\n");
  }
}

static inline ALWAYSINLINE void
tellContext(xid_t ctx)
{
  char		buf[sizeof(xid_t)*3+2];
  size_t	l;

  if (global_args->do_silent) return;

  l = utilvserver_fmt_long(buf,ctx);

  WRITE_MSG(1, "New security context is ");
  write(1, buf, l);
  WRITE_MSG(1, "\n");
}

#include "context-sync.hc"

int main (int argc, char *argv[])
{
  struct Arguments args = {
    .nbctx         = 0,
    .do_disconnect = false,
    .do_silent     = false,
    .flags         = 0,
    .remove_caps   = 0,
    .add_caps      = 0,
    .hostname      = 0,
    .domainname    = 0
  };
  xid_t		newctx;
  int		xflags;
  int		p[2][2];
  pid_t		pid;
  
  global_args = &args;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_DISCONNECT	:  args.do_disconnect = true;   break;
      case CMD_SILENT		:  args.do_silent     = true;   break;
      case CMD_DOMAINNAME	:  args.domainname    = optarg; break;
      case CMD_HOSTNAME		:  args.hostname      = optarg; break;
	
      case CMD_CAP		:
	setCap(optarg, &args.add_caps, &args.remove_caps);
	break;
      case CMD_SECURE		:
	args.remove_caps |= vc_get_insecurecaps();
	break;
      case CMD_FLAG		:
	setFlags(optarg, &args.flags);
	break;
      case CMD_CTX		:
	if (args.nbctx>0)
	  WRITE_MSG(2, "WARNING: More than one ctx not supported by this version\n");
	if (args.nbctx>=DIM_OF(args.ctxs)) {
	  WRITE_MSG(2, "Too many contexts given\n");
	  exit(255);
	}
	args.ctxs[args.nbctx++] = Evc_xidopt2xid(optarg, true);
	break;

	  
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return 255;
	break;
    }
  }

  if (optind>=argc) {
    WRITE_MSG(2, "No command given; use '--help' for more information.\n");
    exit(255);
  }
  
  if (args.domainname && strcmp(args.domainname, "none")==0)
    args.domainname = "";
    
  if (args.nbctx == 0)
    args.ctxs[args.nbctx++] = VC_DYNAMIC_XID;
    
  xflags      = args.flags & S_CTX_INFO_INIT;
  args.flags &= ~S_CTX_INFO_INIT;

  pid = initSync(p, args.do_disconnect);
  if (pid==0) {
    doSyncStage0(p, args.do_disconnect);
    
    newctx            = Evc_new_s_context(args.ctxs[0],0,args.flags);
    args.remove_caps &= (~args.add_caps);
    setHostname(args.hostname);
    setDomainname(args.domainname);

    if (args.remove_caps!=0 || xflags!=0)
      Evc_new_s_context (VC_SAMECTX,args.remove_caps,xflags);
    tellContext(args.ctxs[0]==VC_DYNAMIC_XID ? newctx : args.ctxs[0]);

    doSyncStage1(p, args.do_disconnect);
    execvp (argv[optind],argv+optind);
    doSyncStage2(p, args.do_disconnect);

    PERROR_Q("chcontext: execvp", argv[optind]);
    exit(255);
  }

  waitOnSync(pid, p, args.ctxs[0]!=VC_DYNAMIC_XID);
  return EXIT_SUCCESS;
}

#ifdef ENSC_TESTSUITE
#define FLAG_TEST(STR,EXP) \
  {			   \
    uint32_t	flag=0;	   \
    setFlags(STR, &flag);  \
    assert(flag==(EXP));   \
  }

#define CAP_TEST(STR,EXP_ADD,EXP_DEL)		\
  {						\
    uint32_t	add=0,del=0;			\
    setCap(STR, &add, &del);			\
    assert(add==(EXP_ADD));			\
    assert(del==(EXP_DEL));			\
  }

void
test()
{
  FLAG_TEST("lock",       1);
  FLAG_TEST("lock,sched", 3);

  CAP_TEST("CHOWN",      1, 0);
  CAP_TEST("CAP_CHOWN",  1, 0);
  CAP_TEST("!CHOWN",     0, 1);
  CAP_TEST("!CAP_CHOWN", 0, 1);
}
#endif
