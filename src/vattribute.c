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
#include "attribute-util.h"
#include <lib/vserver.h>

#include <getopt.h>
#include <stdint.h>
#include <errno.h>

#define ENSC_WRAPPERS_PREFIX	"vattribute: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_XID			0x2000
#define CMD_SET			0x2001
#define CMD_CCAP		0x2002
#define CMD_FLAG		0x2003
#define CMD_SECURE		0x2004
#define CMD_BCAP		0x2005
#define CMD_GET			0x2006
#define CMD_UMASK		0x2007

int			wrapper_exit_code = 1;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,       0, CMD_HELP },
  { "version",    no_argument,       0, CMD_VERSION },
  { "xid",        required_argument, 0, CMD_XID },
  { "set",        no_argument,       0, CMD_SET },
  { "get",        no_argument,       0, CMD_GET },
  { "ccap",       required_argument, 0, CMD_CCAP },
  { "bcap",       required_argument, 0, CMD_BCAP },
  { "flag",       required_argument, 0, CMD_FLAG },
  { "secure",     no_argument,       0, CMD_SECURE },
  { "umask",      required_argument, 0, CMD_UMASK },
  {0,0,0,0}
};

struct Arguments {
    xid_t		xid;
    struct vc_ctx_flags flags;
    struct vc_ctx_caps  caps;
    struct vc_umask     umask;
    int			mode;
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n    ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--xid <xid>] {--get|--set [--bcap [~!]<cap>] [--ccap [~!]<cap>]\n"
	    "    [--flag [~!]<flag>] [--secure]} -- [<program> <args>*]\n"
	    "\n"
	    " --bcap <cap>   ...  system  capability to be set\n"
	    " --ccap <cap>   ...  context capability to be set\n"
	    " --flag <flag>  ...  context flag to be set\n"
	    " --umask <mask> ...  unshare mask to be set\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vattribute " VERSION " -- sets/gets attributes of vservers\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
parseFlags(char const *str, struct vc_ctx_flags *flags)
{
  struct vc_err_listparser	err;
  int				rc;

  rc = vc_list2cflag(str,0, &err, flags);
  
  if (rc==-1) {
    WRITE_MSG(2, "Unknown flag '");
    Vwrite(2, err.ptr, err.len);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }
}

static void
parseBCaps(char const *str, struct vc_ctx_caps *caps)
{
  struct vc_err_listparser	err;
  int				rc;

  rc = vc_list2bcap(str,0, &err, caps);
  
  if (rc==-1) {
    WRITE_MSG(2, "Unknown bcap '");
    Vwrite(2, err.ptr, err.len);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }
}

static void
parseCCaps(char const *str, struct vc_ctx_caps *caps)
{
  struct vc_err_listparser	err;
  int				rc;

  rc = vc_list2ccap(str,0, &err, caps);
  
  if (rc==-1) {
    WRITE_MSG(2, "Unknown ccap '");
    Vwrite(2, err.ptr, err.len);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }
}

static void
parseUMask(char const *str, struct vc_umask *umask)
{
  struct vc_err_listparser	err;
  int				rc;

  rc = vc_list2umask(str, 0, &err, umask);
  
  if (rc==-1) {
    WRITE_MSG(2, "Unknown namespace '");
    Vwrite(2, err.ptr, err.len);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }
}

static void
parseSecure(struct vc_ctx_flags UNUSED * flags,
	    struct vc_ctx_caps  UNUSED * caps)
{
  caps->ccaps = ~vc_get_insecureccaps();
  caps->cmask = ~0ull;
  caps->bcaps = ~vc_get_insecurebcaps();
  caps->bmask = ~0ull;

    // TODO: generalize this
  flags->flagword = VC_VXF_HIDE_NETIF;
  flags->mask     = VC_VXF_HIDE_NETIF;
}

static int
printAttrs(struct Arguments *args)
{
  struct vc_ctx_flags flags;
  struct vc_ctx_caps caps;
  struct vc_umask umask = { .mask = ~0, .umask = 0x20200 };

  Evc_get_cflags(args->xid, &flags);
  Evc_get_ccaps(args->xid, &caps);
  Evc_get_umask(args->xid, &umask);

  print_bitfield(1, bcap, "bcapabilities", &caps.bcaps);
  print_bitfield(1, ccap, "ccapabilities", &caps.ccaps);
  print_bitfield(1, cflag, "flags", &flags.flagword);
  print_bitfield(1, umask, "umask", &umask.umask);

  return 0;
}

int main(int argc, char *argv[])
{
  struct Arguments		args = {
    .xid   = VC_NOCTX,
    .flags = { .flagword = 0, .mask = 0 },
    .caps  = { .bcaps = 0, .bmask = 0,.ccaps = 0, .cmask = 0 },
    .umask = { .umask = 0, .mask = 0 },
    .mode  = CMD_SET,
  };

  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_SET	:  args.mode = CMD_SET;                    break;
      case CMD_GET	:  args.mode = CMD_GET;                    break;
      case CMD_XID	:  args.xid = Evc_xidopt2xid(optarg,true); break;
      case CMD_FLAG	:  parseFlags(optarg, &args.flags);        break;
      case CMD_CCAP	:  parseCCaps(optarg, &args.caps);         break;
      case CMD_BCAP	:  parseBCaps(optarg, &args.caps);         break;
      case CMD_SECURE	:  parseSecure(&args.flags, &args.caps);   break;
      case CMD_UMASK	:  parseUMask(optarg, &args.umask);        break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return 255;
	break;
    }
  }

  if (args.xid==VC_NOCTX) args.xid = Evc_get_task_xid(0);

  if (args.mode == CMD_SET) {
    if ((args.caps.cmask || args.caps.bmask) &&
	vc_set_ccaps(args.xid, &args.caps)==-1)
      perror(ENSC_WRAPPERS_PREFIX "vc_set_ccaps()");
    else if (args.flags.mask &&
	     vc_set_cflags(args.xid, &args.flags)==-1)
      perror(ENSC_WRAPPERS_PREFIX "vc_set_flags()");
    else if (args.umask.mask &&
             vc_set_umask(args.xid, &args.umask)==-1)
      perror(ENSC_WRAPPERS_PREFIX "vc_set_umask()");
    else if (optind<argc)
      EexecvpD(argv[optind], argv+optind);
    else
      return EXIT_SUCCESS;
  }
  else if (args.mode == CMD_GET) {
    printAttrs(&args);
    if (optind<argc)
      EexecvpD(argv[optind], argv+optind);
    else
      return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
