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
#include <lib_internal/sys_clone.h>

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
  { "default",    no_argument,       0, 'd' },
  { "~default",   no_argument,       0, 'd' | 0x10000 },

  { "mount",      no_argument,       0, 'M' },
  { "~mount",     no_argument,       0, 'M' | 0x10000 },
  { "fs",         no_argument,       0, 'F' },
  { "~fs",        no_argument,       0, 'F' | 0x10000 },
  { "ipc",        no_argument,       0, 'I' },
  { "~ipc",       no_argument,       0, 'I' | 0x10000 },
  { "uts",        no_argument,       0, 'U' },
  { "~uts",       no_argument,       0, 'U' | 0x10000 },
  { "user",       no_argument,       0, 'S' },
  { "~user",      no_argument,       0, 'S' | 0x10000 },
  { "pid",        no_argument,       0, 'P' },
  { "~pid",       no_argument,       0, 'P' | 0x10000 },
  { "net",        no_argument,       0, 'N' },
  { "~net",       no_argument,       0, 'N' | 0x10000 },
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
	    "    --default         ...  the default spaces for this kernel\n"
	    "    --mount           ...  the mount namespace\n"
	    "    --fs              ...  the fs_struct\n"
	    "    --ipc             ...  the IPC namespace\n"
	    "    --uts             ...  the uts namespace\n"
	    "    --user            ...  the user namespace\n"
	    "    --pid             ...  the pid namespace\n"
	    "    --net             ...  the network namespace\n"
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
  pid_t pid;

  mask &= ~CLONE_FS;

  signal(SIGCHLD, SIG_DFL);

#ifdef NDEBUG
  pid = sys_clone((int) mask | CLONE_VFORK|SIGCHLD, 0);
#else
  pid = sys_clone((int) mask | SIGCHLD, 0);
#endif

  switch (pid) {
    case -1	:
      perror(ENSC_WRAPPERS_PREFIX "clone()");
      exit(wrapper_exit_code);
    case 0	:
      break;
    default	:
      vc_exitLikeProcess(pid, wrapper_exit_code);
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
    int			c = getopt_long(argc, argv, "+nsce:m:" "MFIUSPN", CMDLINE_OPTIONS, 0);
    uint_least64_t	thisbit = 0;
    if (c==-1) break;

    switch (c & 0xFFFF) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case 'n'		:  do_new     = true; break;
      case 's'		:  do_set     = true; break;
      case 'e'		:
	do_enter = true;
	xid      = Evc_xidopt2xid(optarg,true);
	break;
      case 'm'		:  {
	unsigned long	mask_l;
	if (!isNumberUnsigned(optarg, &mask_l, true)) {
	  WRITE_MSG(2, "Invalid mask '");
	  WRITE_STR(2, optarg);
	  WRITE_MSG(2, "'; try '--help' for more information\n");
	  return wrapper_exit_code;
	}
	mask = mask_l;
	break;
      }
      case 'M'		:  thisbit = CLONE_NEWNS;	break;
      case 'F'		:  thisbit = CLONE_FS;		break;
      case 'I'		:  thisbit = CLONE_NEWIPC;	break;
      case 'U'		:  thisbit = CLONE_NEWUTS;	break;
      case 'S'		:  thisbit = CLONE_NEWUSER;	break;
      case 'P'		:  thisbit = CLONE_NEWPID;	break;
      case 'N'		:  thisbit = CLONE_NEWNET;	break;
      case 'd'		:
	thisbit = vc_get_space_default();
	if (thisbit == (__typeof__(thisbit)) -1) {
	  thisbit = vc_get_space_mask();
	  if (thisbit == (__typeof__(thisbit)) -1)
	    thisbit = 0;
	}
	break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return 255;
	break;
    }
    /* ~ option used */
    if (c & 0xFFFF0000)
      mask &= ~thisbit;
    else
      mask |= thisbit;
  }

  sum = ((do_new ? 1 : 0) + (do_enter ? 1 : 0) +
	 (do_set ? 1 : 0));
  
  if (sum==0)
    WRITE_MSG(2, "No operation was specified; try '--help' for more information\n");
  else if (sum>1)
    WRITE_MSG(2, "Can not specify multiple operations; try '--help' for more information\n");
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
