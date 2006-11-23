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
#include <lib_internal/sys_clone.h>

#include <vserver.h>

#include <getopt.h>
#include <libgen.h>
#include <errno.h>
#include <signal.h>
#include <sched.h>

#define ENSC_WRAPPERS_PREFIX	"vnamespace: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#ifndef CLONE_NEWUTS
#  define CLONE_NEWUTS		0x04000000
#endif
#ifndef CLONE_NEWIPC
#  define CLONE_NEWIPC		0x08000000
#endif

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
  { "cleanup",    no_argument,       0, 'c' },
  {0,0,0,0}
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage: ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " <operation> [--] [<program> <args>*]\n"
	    "\n"
	    "<operation> can be one of:\n"
	    "    --new|-n          ...  create new namespace and execute <program> there;\n"
	    "                           <program> is mandatory in this case\n"
	    "    --enter|-e <xid>  ...  enter the namespace of context <xid> and execute\n"
	    "                           <program> there; <program> is mandatory in this\n"
	    "                           case\n"
	    "    --set|-s          ...  make current namespace the namespace of the\n"
	    "                           current context\n"
	    "    --cleanup|-c      ...  remove all mounts from the namespace of the\n"
	    "                           current context\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vnamespace " VERSION " -- manages filesystem-namespace\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
newNamespace(char const *cmd)
{
  pid_t		pid;

  signal(SIGCHLD, SIG_DFL);
  
#ifdef NDEBUG    
  pid = sys_clone(CLONE_NEWNS|CLONE_NEWUTS|CLONE_NEWIPC|CLONE_VFORK|SIGCHLD, 0);
#else
  pid = sys_clone(CLONE_NEWNS|CLONE_NEWUTS|CLONE_NEWIPC|SIGCHLD, 0);
#endif

  switch (pid) {
    case -1	:
      perror("vnamespace: clone()");
      exit(wrapper_exit_code);
    case 0	:
      break;
    default	:
      exitLikeProcess(pid, cmd, wrapper_exit_code);
  }
}

static void
enterNamespace(xid_t xid)
{
  if (vc_enter_namespace(xid)==-1) {
    perror("vnamespace: vc_enter_namespace()");
    exit(255);
  }
}

static void
setNamespace()
{
  if (vc_set_namespace()==-1) {
    perror("vnamespace: vc_set_namespace()");
    exit(255);
  }
}

static void
cleanupNamespace()
{
  if (vc_cleanup_namespace()==-1) {
    perror("vnamespace: vc_cleanup_namespace()");
    exit(255);
  }
}

int main(int argc, char *argv[])
{
  bool		do_new     = false;
  bool		do_enter   = false;
  bool		do_set     = false;
  bool		do_cleanup = false;
  xid_t		xid        = VC_NOCTX;
  int		sum        = 0;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+nsce:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case 'n'		:  do_new     = true; break;
      case 's'		:  do_set     = true; break;
      case 'c'		:  do_cleanup = true; break;
      case 'e'		:
	do_enter = true;
	xid      = Evc_xidopt2xid(optarg,true);
	break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return 255;
	break;
    }
  }

  sum = ((do_new ? 1 : 0) + (do_enter ? 1 : 0) +
	 (do_set ? 1 : 0) + (do_cleanup ? 1 : 0));
  
  if (sum==0)
    WRITE_MSG(2, "No operation was specified; try '--help' for more information\n");
  else if (sum>1)
    WRITE_MSG(2, "Can not specify multiple operations; try '--help' for more information\n");
  else if (optind==argc && (do_new || do_enter))
    WRITE_MSG(2, "No command specified; try '--help' for more information\n");
  else {
    if      (do_new)     newNamespace(argv[optind]);
    else if (do_set)     setNamespace();
    else if (do_cleanup) cleanupNamespace();
    else if (do_enter)   enterNamespace(xid);

    if (optind<argc)
      EexecvpD(argv[optind], argv+optind);

    return EXIT_SUCCESS;
  }

  return 255;
}
