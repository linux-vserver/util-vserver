// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "util.h"
#include "vserver.h"

#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>

#define ENSC_WRAPPERS_PREFIX	"vsched: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_XID			0x4000
#define CMD_FRATE		0x4001
#define CMD_INTERVAL		0x4002
#define CMD_TOKENS		0x4003
#define CMD_TOK_MIN		0x4004
#define CMD_TOK_MAX		0x4005
#define CMD_CPU_MASK		0x4006
#define CMD_PRIO_BIAS		0x4007

int			wrapper_exit_code = 255;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "ctx",           required_argument, 0, CMD_XID },
  { "xid",           required_argument, 0, CMD_XID },
  { "fill-rate",     required_argument, 0, CMD_FRATE },
  { "interval",      required_argument, 0, CMD_INTERVAL },
  { "tokens",        required_argument, 0, CMD_TOKENS },
  { "tokens_min",    required_argument, 0, CMD_TOK_MIN },
  { "tokens-min",    required_argument, 0, CMD_TOK_MIN },
  { "tokens_max",    required_argument, 0, CMD_TOK_MAX },
  { "tokens-max",    required_argument, 0, CMD_TOK_MAX },
  { "prio_bias",     required_argument, 0, CMD_PRIO_BIAS },
  { "prio-bias",     required_argument, 0, CMD_PRIO_BIAS },
  { "priority_bias", required_argument, 0, CMD_PRIO_BIAS },
  { "priority-bias", required_argument, 0, CMD_PRIO_BIAS },
  { "cpu_mask",      required_argument, 0, CMD_CPU_MASK },
  {0,0,0,0}
};

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);

  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--xid <xid>] [--fill-rate <rate>] [--interval <interval>] [--tokens <tokens>] [--tokens-min <tokens>] [--tokens-max <tokens>] [--prio-bias <bias>] [--] [<command> <args>*]\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vsched " VERSION " -- modifies scheduling parameters\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

#define SETVAL(ATTR,MASK) \
  sched.ATTR      = atoi(optarg); \
  sched.set_mask |= MASK;

int main(int argc, char *argv[])
{
  xid_t			xid   = VC_NOCTX;
  struct vc_set_sched	sched = {
    .set_mask = 0
  };
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_XID	:  xid = Evc_xidopt2xid(optarg,true);         break;
      case CMD_FRATE	:  SETVAL(fill_rate,     VC_VXSM_FILL_RATE);  break;
      case CMD_INTERVAL	:  SETVAL(interval,      VC_VXSM_INTERVAL);   break;
      case CMD_TOKENS	:  SETVAL(tokens,        VC_VXSM_TOKENS);     break;
      case CMD_TOK_MIN	:  SETVAL(tokens_min,    VC_VXSM_TOKENS_MIN); break;
      case CMD_TOK_MAX	:  SETVAL(tokens_max,    VC_VXSM_TOKENS_MAX); break;
      case CMD_PRIO_BIAS:  SETVAL(priority_bias, VC_VXSM_PRIO_BIAS);  break;
      case CMD_CPU_MASK	:
	WRITE_MSG(2, "vsched: WARNING: the '--cpu_mask' parameter is deprecated and will not have any effect\n");
	break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (xid==VC_NOCTX && optind==argc) {
    WRITE_MSG(2, "Without a program, '--xid' must be used; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  if (sched.set_mask==0 && optind==argc) {
    WRITE_MSG(2, "Neither an option nor a program was specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  if (xid==VC_NOCTX)
    xid = Evc_get_task_xid(0);

  if (sched.set_mask!=0 && vc_set_sched(xid, &sched)==-1) {
    perror("vc_set_sched()");
    exit(255);
  }

  if (optind<argc)
    EexecvpD(argv[optind],argv+optind);

  return EXIT_SUCCESS;
}
