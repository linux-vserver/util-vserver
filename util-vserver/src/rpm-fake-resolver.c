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

#include "internal.h"
#include "vserver.h"
#include "util.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <grp.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>

#define ENSC_WRAPPERS_PREFIX	"rpm-fake-resolver: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#include <wrappers.h>

#define MAX_RQSIZE	0x1000

int wrapper_exit_code = 1;

struct ArgInfo {
    xid_t		ctx;
    uid_t		uid;
    gid_t		gid;
    bool		do_fork;
    char const *	pid_file;
    char const *	chroot;
};

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
	    " [-c <ctx>] [-u <uid>] [-g <gid>] [-r <chroot>] [-n]\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "rpm-fake-resolver " VERSION " -- NSS resovler for rpm-fake\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}


inline static void
parseArgs(struct ArgInfo *args, int argc, char *argv[])
{
  while (1) {
    int		c = getopt_long(argc, argv, "c:u:g:r:n", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case 'v'		:  showVersion();

      case 'c'		:  args->ctx = atoi(optarg); break;
      case 'u'		:  args->uid = atoi(optarg); break;
      case 'g'		:  args->gid = atoi(optarg); break;
      case 'r'		:  args->chroot  = optarg;   break;
      case 'n'		:  args->do_fork = false;    break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	exit(1);
	break;
    }
  }

  if (optind!=argc) {
    WRITE_MSG(2, "No further options allowed; aborting ...\n");
    exit(1);
  }
  
  if (args->chroot==0) {
    WRITE_MSG(2, "No chroot specified; aborting...\n");
    exit(1);
  }
}

static void
sendResult(bool state, uint32_t res)
{
  if (state) {
    static uint8_t	ONE = 1;
    Ewrite(1, &ONE, sizeof ONE);
  }
  else {
    static uint8_t	ZERO = 0;
    Ewrite(1, &ZERO, sizeof ZERO);
  }

  Ewrite(1, &res, sizeof res);
}

static void
do_getpwnam()
{
  uint32_t	len;
  Eread(0, &len, sizeof len);

  if (len<MAX_RQSIZE) {
    char		buf[len+1];
    struct passwd *	res;
    
    Eread(0, buf, len);
    buf[len] = '\0';
    res = getpwnam(buf);
    if (res!=0) sendResult(true,  res->pw_uid);
    else        sendResult(false, -1);
  }
  // TODO: logging
}

static void
do_getgrnam()
{
  uint32_t	len;
  Eread(0, &len, sizeof len);

  if (len<MAX_RQSIZE) {
    char		buf[len+1];
    struct group *	res;
    
    Eread(0, buf, len);
    buf[len] = '\0';
    res = getgrnam(buf);
    if (res!=0) sendResult(true,  res->gr_gid);
    else        sendResult(false, -1);
  }
  // TODO: logging
}

static void
do_closenss()
{
  uint8_t	what;
  Eread(0, &what, sizeof what);
  switch (what) {
    case 'p'	:  endpwent(); break;
    case 'g'	:  endgrent(); break;
    default	:  break;
  }
}

static void
run()
{
  uint8_t	c;

  while (true) {
    Ewrite(3, ".", 1);
    Eread (0, &c,  sizeof c);
    switch (c) {
      case 'P'	:  do_getpwnam(); break;
      case 'G'	:  do_getgrnam(); break;
      case 'Q'	:  exit(0);
      case 'C'	:  do_closenss(); break;
      case '.'	:  Ewrite(1, ".", 1); break;
      default	:  Ewrite(1, "?", 1); break;
    }
  }
}

static void
daemonize(struct ArgInfo const UNUSED * args, int pid_fd)
{
  int		p[2];
  pid_t		pid;
  char		c;
  
  Epipe(p);
  pid = Efork();
  
  if (pid!=0) {
    if (pid_fd!=-1) {
      char	buf[sizeof(id_t)*3 + 2];
      size_t	l;

      l = utilvserver_fmt_uint(buf, pid);
      Ewrite(pid_fd, buf, l);
      Ewrite(pid_fd, "\n", 1);
    }
    _exit(0);
  }
  Eclose(p[1]);
  read(p[0], &c, 1);
  Eclose(p[0]);
}

int main(int argc, char * argv[])
{
  struct ArgInfo	args = {
    .ctx      = VC_DYNAMIC_XID,
    .uid      = 99,
    .gid      = 99,
    .do_fork  = true,
    .pid_file = 0,
    .chroot   = 0
  };
  int			pid_fd = -1;

#ifndef __dietlibc__
#  warning  *** rpm-fake-resolver is built against glibc; please do not report errors before trying a dietlibc version ***
  WRITE_MSG(2,
	    "***  rpm-fake-resolver was built with glibc;  please do  ***\n"
	    "***  not report errors before trying a dietlibc version. ***\n");
#endif

  parseArgs(&args, argc, argv);
  if (args.pid_file && args.do_fork)
    pid_fd = EopenD(args.pid_file, O_CREAT|O_WRONLY, 0644);
  
  if (args.chroot) Echroot(args.chroot);
  Echdir("/");

  //Evc_new_s_context(args.ctx, ~(VC_CAP_SETGID|VC_CAP_SETUID), S_CTX_INFO_LOCK);
  Evc_new_s_context(args.ctx, 0, S_CTX_INFO_LOCK);
  Esetgroups(0, &args.gid);
  Esetgid(args.gid);
  Esetuid(args.uid);

  if (args.do_fork) daemonize(&args, pid_fd);
  if (pid_fd!=-1)   close(pid_fd);
  run();
}
