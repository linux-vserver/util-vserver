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

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--] <pipe> <timeout>\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vshelper-sync " VERSION " -- waits for data from a pipe"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static volatile sig_atomic_t	timeout_flag = 0;

void alarmHandler(int UNUSED sig)
{
  timeout_flag = 1;
}

int main(int argc, char *argv[])
{
  int		fd;
  int		idx = 1;

  if (argc>=2) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(1, argv[0], 0);
    if (strcmp(argv[1], "--version")==0) showVersion();
    if (strcmp(argv[1], "--")       ==0) ++idx;
  }

  if (argc<idx+2) {
    WRITE_MSG(2, "Not enough parameters; use '--help' for more information\n");
    return EXIT_FAILURE;
  }


  signal(SIGALRM, alarmHandler);
  alarm(atoi(argv[idx+1]));
  
  fd = open(argv[idx], O_RDONLY,0);
  if (timeout_flag) return EXIT_FAILURE;
  if (fd==-1) {
    perror("vshelper-sync: open()");
    return EXIT_FAILURE;
  }
  
  for (;;) {
    char	buf[512];
    ssize_t	len = read(fd,buf,sizeof buf);
    if (len==0) break;
    if (timeout_flag) return EXIT_FAILURE;
    if (len==-1) {
      perror("vshelper-sync: read()");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
