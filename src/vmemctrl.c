// $Id$

// Copyright (C) 2007 Daniel Hokka Zakrisson
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.h"
#include "util.h"

#include <lib/internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#define ENSC_WRAPPERS_PREFIX	"vmemctrl: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#include "wrappers.h"

#define CMD_HELP	0x1000
#define CMD_VERSION	0x1001

#define CMD_SET		0x2000
#define CMD_GET		0x2001

#define CMD_XID		0x4000
#define CMD_BADNESS	0x4001

int wrapper_exit_code = 255;


static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,        0, CMD_HELP },
  { "version",  no_argument,        0, CMD_VERSION },
  { "set",      no_argument,        0, CMD_SET },
  { "get",      no_argument,        0, CMD_GET },
  { "xid",      required_argument,  0, CMD_XID },
  { "badness",  required_argument,  0, CMD_BADNESS },
  { 0,0,0,0 }
};

struct Arguments {
  xid_t		xid;
  int64_t	badness;
  bool		do_set;
  bool		do_get;
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " (--set|--get) [--xid <xid>] [--badness <OOM bias>]\n"
	    "        [--] [<command> <args>*]\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vmemctrl " VERSION " -- \n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2007 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static inline void
doset(struct Arguments *args)
{
  if (vc_set_badness(args->xid, args->badness) == -1) {
    perror(ENSC_WRAPPERS_PREFIX "vc_set_badness()");
    exit(wrapper_exit_code);
  }
}

static inline void
doget(struct Arguments *args)
{
  int64_t badness;
  char buf[32];
  size_t l;
  if (vc_get_badness(args->xid, &badness) == -1) {
    perror(ENSC_WRAPPERS_PREFIX "vc_get_badness()");
    exit(wrapper_exit_code);
  }
  l = utilvserver_fmt_int64(buf, badness);
  buf[l] = '\0';
  WRITE_STR(1, buf);
  WRITE_MSG(1, "\n");
}

int main (int argc, char *argv[])
{
  struct Arguments args = {
    .do_set	= false,
    .do_get	= false,
    .xid	= VC_NOCTX,
    .badness	= 0,
  };
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_XID	:  args.xid       = Evc_xidopt2xid(optarg,true); break;
      case CMD_SET	:  args.do_set    = true; break;
      case CMD_GET	:  args.do_get    = true; break;
      case CMD_BADNESS	: {
	char *endptr;
	args.badness = strtoll(optarg, &endptr, 0);
	if (*endptr) {
	  WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Badness '");
	  WRITE_STR(2, optarg);
	  WRITE_MSG(2, "' is not an integer\n");
	  exit(wrapper_exit_code);
	}
	break;
      }
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	exit(wrapper_exit_code);
	break;
    }
  }

  if (args.xid == VC_NOCTX) args.xid = Evc_get_task_xid(0);

  if (!args.do_set && !args.do_get) {
    WRITE_MSG(2, "No operation specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }
  else if (((args.do_set ? 1 : 0) + (args.do_get ? 1 : 0)) > 1) {
    WRITE_MSG(2, "Multiple operations specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  if (args.do_set)
    doset(&args);
  else if (args.do_get)
    doget(&args);

  if (optind != argc)
    Eexecvp (argv[optind],argv+optind);
  return EXIT_SUCCESS;
}
