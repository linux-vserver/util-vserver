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
#include "wrappers.h"
#include "stack-start.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>

#ifndef CLONE_NEWNS
#  define CLONE_NEWNS 0x00020000
#endif

int	wrapper_exit_code = 255;

static int
childFunc(void *argv_v)
{
  char **	argv = argv_v;
  
  execvp(argv[0], argv);
  perror("execvp()");
  exit(255);
}

static void
usage(int fd, char const *cmd, int exit_code)
{
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
  char			buf[128];
  int			status;
  pid_t			pid, p;

  if (argc==1) usage(2, argv[0], 255);
  if (!strcmp(argv[1], "--help"))    usage(1, argv[0], 0);
  if (!strcmp(argv[1], "--version")) showVersion();
  if (!strcmp(argv[1], "--"))        ++argv;

  pid = Eclone(childFunc, STACK_START(buf), CLONE_NEWNS|CLONE_VFORK|SIGCHLD, argv+1);
  p   = Ewait4(pid, &status, 0,0);

  if (WIFEXITED(status)) return WEXITSTATUS(status);
  else                   return 255;
}
