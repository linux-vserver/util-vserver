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
#include <vserver.h>

#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>

static void
showHelp(int fd, char const *cmd, int exit_code)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd,	"Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " <xid> <cmd> <args*>\n"
	    "\n"
	    "Enters namespace of context <xid> and executes <cmd> there.\n"
	    "\n"
	    "Report bugs to " PACKAGE_BUGREPORT "\n");

  exit(exit_code);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "enter-namespace " VERSION " -- enters namespaces and executes programs there\n"
	    "This program is part of " PACKAGE_STRING "\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main(int argc, char *argv[])
{
  if (argc==1) showHelp(2, argv[0], 255);
  if (!strcmp(argv[1], "--help"))    showHelp(1, argv[0], 0);
  if (!strcmp(argv[1], "--version")) showVersion();
  if (!strcmp(argv[1], "--"))        { ++argv; --argc; }

  if (argc<2) {
    WRITE_MSG(2, "No context and/or command specified; try '--help' for more information\n");
    exit(255);
  }

  if (vc_enter_namespace(atoi(argv[1]))==-1) {
    perror("enter-namespace: vc_enter_namespace()");
    exit(255);
  }

  execvp(argv[2], argv+2);
  perror("enter-namespace: execvp()");
  exit(255);
}
