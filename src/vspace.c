// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// Copyright (C) 2007 Daniel Hokka Zakrisson
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
#include <lib_internal/sys_unshare.h>

#include <vserver.h>

#include <getopt.h>
#include <libgen.h>
#include <errno.h>
#include <signal.h>
#include <sched.h>

#define ENSC_WRAPPERS_PREFIX	"vspace: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001

int		wrapper_exit_code  =  255;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,       0, CMD_HELP },
  { "version",    no_argument,       0, CMD_VERSION },
  { "new",        no_argument,       0, 'n' },
  { "enter",      required_argument, 0, 'e' },
  { "set",        no_argument,       0, 's' },
  { "mask",       required_argument, 0, 'm' },

  { "mount",      no_argument,       0, 'M' },
  { "fs",         no_argument,       0, 'F' },
  { "ipc",        no_argument,       0, 'I' },
  { "uts",        no_argument,       0, 'U' },
  { "user",       no_argument,       0, 'S' },
  {0,0,0,0}
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage: ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " <operation> <spaces>* [--] [<program> <args>*]\n"
	    "\n"
	    "<operation> can be one of:\n"
	    "    --new|-n          ...  create new spaces and execute <program> there;\n"
	    "                           <program> is mandatory in this case\n"
	    "    --enter|-e <xid>  ...  enter the spaces of context <xid> and execute\n"
	    "                           <program> there; <program> is mandatory in this\n"
	    "                           case\n"
	    "    --set|-s          ...  assign the current spaces to the current context\n"
	    "\n"
	    "<spaces>* specifies the spaces to manipulate.\n"
	    "It can be any combination of:\n"
	    "    --mask <mask>     ...  specify a mask of spaces\n"
	    "    --mount           ...  the mount namespace\n"
	    "    --fs              ...  the fs_struct\n"
	    "    --ipc             ...  the IPC namespace\n"
	    "    --uts             ...  the uts namespace\n"
	    "    --user            ...  the user namespace\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vspace " VERSION " -- manages spaces\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    "Copyright (C) 2007 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
newSpaces(uint_least64_t mask)
{
  if (sys_unshare((int) mask) == -1) {
    perror(ENSC_WRAPPERS_PREFIX "unshare()");
    exit(wrapper_exit_code);
  }
}

static void
enterSpaces(xid_t xid, uint_least64_t mask)
{
  if (vc_enter_namespace(xid, mask)==-1) {
    perror(ENSC_WRAPPERS_PREFIX "vc_enter_namespace()");
    exit(wrapper_exit_code);
  }
}

static void
setSpaces(xid_t xid, uint_least64_t mask)
{
  if (vc_set_namespace(xid, mask)==-1) {
    perror(ENSC_WRAPPERS_PREFIX "vc_set_namespace()");
    exit(wrapper_exit_code);
  }
}

int main(int argc, char *argv[])
{
  bool			do_new     = false;
  bool			do_enter   = false;
  bool			do_set     = false;
  uint_least64_t	mask       = 0;
  xid_t			xid        = VC_NOCTX;
  int			sum        = 0;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+nsce:m:" "MFIUS", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case 'n'		:  do_new     = true; break;
      case 's'		:  do_set     = true; break;
      case 'e'		:
	do_enter = true;
	xid      = Evc_xidopt2xid(optarg,true);
	break;
      case 'm'		:
	if (!isNumberUnsigned(optarg, &mask, true)) {
	  WRITE_MSG(2, "Invalid mask '");
	  WRITE_STR(2, optarg);
	  WRITE_MSG(2, "'; try '--help' for more information\n");
	  return wrapper_exit_code;
	}
	break;
      case 'M'		:  mask |= CLONE_NEWNS;		break;
      case 'F'		:  mask |= CLONE_FS;		break;
      case 'I'		:  mask |= CLONE_NEWIPC;	break;
      case 'U'		:  mask |= CLONE_NEWUTS;	break;
      case 'S'		:  mask |= CLONE_NEWUSER;	break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return 255;
	break;
    }
  }

  sum = ((do_new ? 1 : 0) + (do_enter ? 1 : 0) +
	 (do_set ? 1 : 0));
  
  if (sum==0)
    WRITE_MSG(2, "No operation was specified; try '--help' for more information\n");
  else if (sum>1)
    WRITE_MSG(2, "Can not specify multiple operations; try '--help' for more information\n");
  else if (mask==0)
    WRITE_MSG(2, "Must specify at least one space; try '--help' for more information\n");
  else if (optind==argc && (do_new || do_enter))
    WRITE_MSG(2, "No command specified; try '--help' for more information\n");
  else {
    if      (do_new)     newSpaces(mask);
    else if (do_set)     setSpaces(VC_SAMECTX, mask);
    else if (do_enter)   enterSpaces(xid, mask);

    if (optind<argc)
      EexecvpD(argv[optind], argv+optind);

    return EXIT_SUCCESS;
  }

  return 255;
}
