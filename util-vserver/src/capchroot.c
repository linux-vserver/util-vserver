// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on capchroot.cc by Jacques Gelinas
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
	This chroot command does very little. Once the chroot
	system call is executed, it (option) remove the CAP_SYS_CHROOT
	capability. Then it executes its argument
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <libgen.h>

#define ENSC_WRAPPERS_PREFIX	"capchroot: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_NOCHROOT		0x2000
#define CMD_SUID		0x2001

int			wrapper_exit_code = 255;

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",        no_argument,       0, CMD_HELP },
  { "version",     no_argument,       0, CMD_VERSION },
  { "nochroot",    no_argument,       0, CMD_NOCHROOT },
  { "suid",        required_argument, 0, CMD_SUID },
  {0,0,0,0}
};

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --nochroot [--suid <user>] [--] <directory> <command> <args>*\n"
	    "\n"
	    "Options:\n"
            "    --nochroot     ... remove the CAP_SYS_CHROOT capability\n"
            "                       after the chroot system call.\n"
	    "    --suid <user>  ... switch to a different user (in the vserver\n"
	    "                       context) before executing the command.\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "capchroot " VERSION " -- a capability aware chroot\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
setUser(char const *user)
{
  struct passwd		*p = 0;
  if (user!=0 && strcmp(user, "root")!=0) {
    errno = 0;
    p     = getpwnam(user);
    if (p==0) {
      if (errno==0) errno = ENOENT;
      PERROR_Q(ENSC_WRAPPERS_PREFIX "getpwnam", user);
      exit(wrapper_exit_code);
    }
  }

  if (p!=0) {
    Esetgroups(1, &p->pw_gid);
    Esetgid(p->pw_gid);
    Esetuid(p->pw_uid);

    if (getuid()!=p->pw_uid || getgid()!=p->pw_gid) {
      WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Something went wrong while changing uid; expected uid/gid do not match the actual one\n");
      exit(wrapper_exit_code);
    }
  }
}
    
int main (int argc, char *argv[])
{
  bool 			nochroot  = false;
  char const *		suid_user = 0;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_NOCHROOT	:  nochroot  = true;   break;
      case CMD_SUID	:  suid_user = optarg; break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (optind==argc)
    WRITE_MSG(2, "No directory specified; try '--help' for more information\n");
  else if (optind+1==argc)
    WRITE_MSG(2, "No command specified; try '--help' for more information\n");
  else {
      // We resolve the UID before doing the chroot.
      // If we do the getpwnam after the chroot, we will end
      // up loading shared object from the vserver.
      // This is causing two kind of problem: Incompatibilities
      // and also a security flaw. The shared objects in the vserver
      // may be tweaked to get control of the root server ...
    getpwnam("root");
    Echroot(argv[optind]);
    if (nochroot)
      Evc_new_s_context(VC_SAMECTX, 1<<VC_CAP_SYS_CHROOT,0);
    setUser(suid_user);
    EexecvpD(argv[optind+1], argv+optind+1);
  }

  return EXIT_FAILURE;
}
