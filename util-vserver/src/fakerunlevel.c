// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on fakerunlevel.cc by Jacques Gelinas
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
	This program add a RUNLEVEL record in a utmp file.
	This is used when a vserver lack a private init process
	so runlevel properly report the fake runlevel.
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util.h"

#include <utmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>


static void
showHelp(int fd, int exit_code)
{
  WRITE_MSG(fd,
	    "Usage: fakerunlevel <runlevel> <utmp_file>\n\n"
	    "Put a runlevel record in file <utmp_file>\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(exit_code);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "fakerunlevel " VERSION "\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main (int argc, char *argv[])
{
  if (argc==1) showHelp(2,1);
  if (strcmp(argv[1], "--help")==0)    showHelp(1,0);
  if (strcmp(argv[1], "--version")==0) showVersion();
  if (argc!=3) showHelp(2,1);

  {
    int  const 		runlevel = atoi(argv[1]);
    char const * const	fname    = argv[2];
    int			fd;
    struct utmp 	ut;
    
    gid_t		gid;
    char		*gid_str = getenv("UTMP_GID");
    
    if (runlevel<0 || runlevel>6) showHelp(2,1);

    if (chroot(".")==-1 ||
	chdir("/")==-1) {
      perror("chroot()/chdir()");
      return EXIT_FAILURE;
    }

      // Real NSS is too expensive/insecure in this state; therefore, use the
      // value detected at ./configure stage or overridden by $UTMP_GID
      // env-variable
    gid = gid_str ? atoi(gid_str) : UTMP_GID;
    if (setgroups(1,&gid)==-1 ||
	setgid(gid)==-1 ||
	getgid()!=gid) {
      perror("setgid()/getgid()");
      return EXIT_FAILURE;
    }

    umask(002);
    fd = open(fname, O_WRONLY|O_CREAT|O_APPEND, 0664);
    if (fd==-1) {
      perror("open()");
      return EXIT_FAILURE;
    }

    if (close(fd)==-1) {
      perror("close()");
      return EXIT_FAILURE;
    }

    utmpname (fname);
    setutent();
    memset (&ut,0,sizeof(ut));
    ut.ut_type = RUN_LVL;
    ut.ut_pid  = ('#' << 8) + runlevel+'0';
    pututline (&ut);
    endutent();
  }

  return EXIT_SUCCESS;
}
