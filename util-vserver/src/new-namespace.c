// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

// Executes a program in a new namespace
// Based on http://www.win.tue.nl/~aeb/linux/lk/lk-6.html


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util.h"
#include "stack-start.h"
#include "sys_clone.h"

#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>

#define ENSC_WRAPPERS_WAIT	1
#include <wrappers.h>

#ifndef CLONE_NEWNS
#  define CLONE_NEWNS 0x00020000
#endif

int	wrapper_exit_code = 255;

static void
showHelp(int fd, char const *cmd, int exit_code)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd,	"Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--help] [--version] <cmd> <args*>\n"
	    "\n"
	    "Executes <cmd> in a new namespace.\n"
	    "\n"
	    "Report bugs to " PACKAGE_BUGREPORT "\n");

  exit(exit_code);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "new-namespace " VERSION " -- executes programs in a new namespace\n"
	    "This program is part of " PACKAGE_STRING "\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main(int argc, char *argv[])
{
  pid_t			pid;

  if (argc==1) showHelp(2, argv[0], 255);
  if (!strcmp(argv[1], "--help"))    showHelp(1, argv[0], 0);
  if (!strcmp(argv[1], "--version")) showVersion();
  if (!strcmp(argv[1], "--"))        ++argv;

  
#ifdef NDEBUG    
  pid = sys_clone(CLONE_NEWNS|CLONE_VFORK|SIGCHLD, 0);
#else
  pid = sys_clone(CLONE_NEWNS|SIGCHLD, 0);
#endif
  switch (pid) {
    case -1	:
      perror("clone()");
      exit(wrapper_exit_code);
    case 0	:
      execvp(argv[1], argv+1);
      perror("execvp()");
      exit(wrapper_exit_code);
    default	:
      exitLikeProcess(pid);
  }
}
