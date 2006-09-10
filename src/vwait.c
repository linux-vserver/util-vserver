// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "lib/vserver.h"
#include "lib/internal.h"
#include "util.h"

#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <libgen.h>
#include <assert.h>

#define ENSC_WRAPPERS_PREFIX	"vwait: "
#define ENSC_WRAPPERS_STDLIB	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#define CMD_HELP	0x8000
#define CMD_VERSION	0x8001

#define CMD_TIMEOUT	0x4000
#define CMD_TERMINATE	0x4001
#define CMD_STATUS_FD	0x4002

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,        0, CMD_HELP },
  { "version",    no_argument,        0, CMD_VERSION },
  { "timeout",    required_argument,  0, CMD_TIMEOUT },
  { "terminate",  no_argument,        0, CMD_TERMINATE },
  { "status-fd",  required_argument,  0, CMD_STATUS_FD },
  { 0,0,0,0 }
};

int			wrapper_exit_code = 1;
static sig_atomic_t	aborted = 0;

struct StatusType {
    enum {stERROR, stFINISHED, stKILLED,
	  stTIMEOUT}				status;
    int						rc;
};

struct Arguments
{
    xid_t		xid;
    int			timeout;
    int			status_fd;
    bool		do_terminate;
};

static void
showHelp(char const *cmd)
{
  VSERVER_DECLARE_CMD(cmd);

  WRITE_MSG(1, "Usage:  ");
  WRITE_STR(1, cmd);
  WRITE_STR(1,
	    " [--timeout <timeout>] [--terminate] [--status-fd <fd>] [--] <xid>\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vwait " VERSION " -- waits for a context to finish\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2005 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
handler(int UNUSED num)
{
  aborted = 1;
}

static struct StatusType
doit(struct Arguments const *args)
{
  time_t			end_time = 0, now = 0;
  struct StatusType		res;
  
  if (args->timeout>0) {
    end_time = time(0) + args->timeout;
    siginterrupt(SIGALRM, 1);
    signal(SIGALRM, handler);
    alarm(args->timeout);
  }

  for (;;) {
    res.rc = vc_wait_exit(args->xid);
    
    if      (res.rc==-1 && errno!=EAGAIN && errno!=EINTR) {
	// the error-case
      res.rc     = errno;
      res.status = stERROR;
      perror(ENSC_WRAPPERS_PREFIX "vc_wait_exit()");
    }
    else if (res.rc==-1 && args->timeout>0 && (now=time(0))>=end_time) {
	// an EINTR or EAGAIN signal was delivered, a timeout was set and
	// reached
      if (!args->do_terminate)
	res.status = stTIMEOUT;
      else {
	vc_ctx_kill(args->xid, 1, 9);
	vc_ctx_kill(args->xid, 0, 9);
	res.status = stKILLED;
      }
    }
    else if (res.rc==-1) {
	// an EINTR or EAGAIN signal was delivered but the timeout not set or
	// not reached yet

	// we are here, when args->timeout==0 or 'now' was initialized (and
	// compared with 'end_time'). So, 'now' can be used below.
      assert(args->timeout<=0 || (now < end_time));

      if (args->timeout>0)	// (re)set the alarm-clock
	alarm(end_time-now);

      continue;
    }
    else
	// vc_wait_exit(2) finished successfully
      res.status = stFINISHED;

    break;
  }

  alarm(0);
  return res;
}

static void
writeStatus(int fd, char const *str, int const *rc, int exit_code)
{
  if (fd==-1) exit(exit_code);

  WRITE_STR(fd, str);
  if (rc) {
    char		buf[sizeof(*rc)*3 + 2];
    size_t		len = utilvserver_fmt_long(buf, *rc);
    WRITE_MSG(fd, " ");
    Vwrite   (fd, buf, len);
  }
  WRITE_MSG(fd, "\n");
  
  exit(exit_code);
}

int main(int argc, char *argv[])
{
  struct StatusType	res;
  struct Arguments	args = {
    .xid          = VC_NOCTX,
    .timeout      = -1,
    .status_fd    = -1,
    .do_terminate = false,
  };

  while (1) {
    int		c = getopt_long(argc, argv, "c:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(argv[0]);
      case CMD_VERSION		:  showVersion();
      case CMD_TERMINATE	:  args.do_terminate = true;         break;
      case CMD_TIMEOUT		:  args.timeout      = atoi(optarg); break;
      case CMD_STATUS_FD	:  args.status_fd    = atoi(optarg); break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (optind+1 > argc) {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "no context specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  if (optind+1 < argc) {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "can not wait for more than one context; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }
  
  args.xid = Evc_xidopt2xid(argv[optind], true);

  if (args.xid==VC_NOCTX) {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "invalid context specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  res = doit(&args);

  switch (res.status) {
    case stERROR	:  writeStatus(args.status_fd, "ERROR",    &res.rc, 127);
    case stFINISHED	:  writeStatus(args.status_fd, "FINISHED", &res.rc,   0);
    case stKILLED	:  writeStatus(args.status_fd, "KILLED",         0,   1);
    case stTIMEOUT	:  writeStatus(args.status_fd, "TIMEOUT",        0,   2);
    default		:  writeStatus(args.status_fd, "???",      &res.rc, 126);
  }
}
