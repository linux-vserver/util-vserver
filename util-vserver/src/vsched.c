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

int			wrapper_exit_code = 255;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "ctx",         required_argument, 0, CMD_XID },
  { "fill-rate",   required_argument, 0, CMD_FRATE },
  { "interval",    required_argument, 0, CMD_INTERVAL },
  { "tokens",      required_argument, 0, CMD_TOKENS },
  { "tokens_min",  required_argument, 0, CMD_TOK_MIN },
  { "tokens_max",  required_argument, 0, CMD_TOK_MAX },
  { "cpu_mask",    required_argument, 0, CMD_CPU_MASK },
  {0,0,0,0}
};

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);

  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--ctx <xid>] [--fill-rate <rate>] [--interval <interval>] [--tokens <tokens>] [--tokens_min <tokens>] [--tokens_max <tokens>] [--cpu_mask <mask>] [--] [<command> <args>*]\n"
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

int main(int argc, char *argv[])
{
  xid_t			xid   = VC_NOCTX;
  struct vc_set_sched	sched = { 0,0,0,0,0,0 };
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_XID	:  xid = atoi(optarg); break;
      case CMD_FRATE	:  sched.fill_rate   = atoi(optarg); break;
      case CMD_INTERVAL	:  sched.interval    = atoi(optarg); break;
      case CMD_TOKENS	:  sched.tokens      = atoi(optarg); break;
      case CMD_TOK_MIN	:  sched.tokens_min  = atoi(optarg); break;
      case CMD_TOK_MAX	:  sched.tokens_max  = atoi(optarg); break;
      case CMD_CPU_MASK	:  sched.cpu_mask    = atoi(optarg); break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (xid==VC_NOCTX && optind==argc) {
    WRITE_MSG(2, "Neither '--xid' nor a program was specified; try '--help' for more information\n");
    exit(255);
  }

  if (xid==VC_NOCTX)
    xid = Evc_get_task_xid(0);

  if (vc_set_sched(xid, &sched)==-1) {
    perror("vc_set_sched()");
    exit(255);
  }

  EexecvpD(argv[optind],argv+optind);
}
