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

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <sys/file.h>

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--] <lockfile> <syncpipe> [<timeout>]\n\n"
	    "Protocol:\n"
	    "  1.  parent (shell) creates a named <syncpipe>\n"
	    "  2.  'lockfile' will be called\n"
	    "  3a. 'lockfile' waits until somebody opens the <syncpipe> for reading\n"
	    "  3b. parent (shell) opens the pipe for reading and blocks\n"
	    "  4.  'lockfile' calls flock() on the <lockfile>\n"
	    "  5.  'lockfile' closes the <syncpipe>\n"
	    "  6.  parent (shell) unlocks since <syncpipe> is closed\n"
	    "  7.  'lockfile' goes into infinite loop\n"
	    "  8.  parent sends SIGHUP (or other signal) to 'lockfile\n"
	    "\n"
	    "Sample code:\n"
	    "  tmp=$(mktemp /tmp/lock.XXXXXX)\n"
	    "  rm -f $tmp    # safe since mknod(2) does not follow symlinks\n"
	    "  mkfifo -m600 $tmp\n"
	    "  lockfile $lock $tmp &\n"
	    "  cat <$tmp >/dev/null\n"
	    "  ... <actions> ...\n"
	    "  kill -HUP $!  # (implicated by shell-exit)\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "lockfile " VERSION " -- locks a file"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static volatile int		timeout_flag = 0;

static void
alarmFunc(int UNUSED sig)
{
  timeout_flag = 1;
  signal(SIGALRM, alarmFunc);
}

static void
quitFunc(int UNUSED sig)
{
  _exit(0);
}

int main(int argc, char *argv[])
{
  int			fd, sync_fd;
  int			idx = 1;
  time_t		end_time;

  if (argc>=2) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(1, argv[0], 0);
    if (strcmp(argv[1], "--version")==0) showVersion();
    if (strcmp(argv[1], "--")       ==0) ++idx;
  }

  if (argc<idx+2) {
    WRITE_MSG(2, "Not enough parameters; use '--help' for more information\n");
    return EXIT_FAILURE;
  }

  end_time = time(0);
  if (argc==idx+3) end_time += atoi(argv[idx+2]);
  else             end_time += 300;

  
		   
  if ((fd=open(argv[idx], O_CREAT|O_RDONLY|O_NOFOLLOW|O_NONBLOCK, 0644))==-1)
    perror("lockfile: open(<lockfile>)");
  else if ((sync_fd=open(argv[idx+1], O_WRONLY))==-1)
    perror("lockfile: open(<syncpipe>)");
  else if (unlink(argv[idx+1])==-1)
    perror("lockfile: unlink(<syncpipe>)");
  else if (siginterrupt(SIGALRM, 1)==-1)
    perror("lockfile: siginterrupt()");
  else if (signal(SIGALRM, alarmFunc)==SIG_ERR ||
	   signal(SIGHUP,  quitFunc)==SIG_ERR)
    perror("lockfile: signal()");
  else while (time(0)<end_time && getppid()!=1) {
    alarm(1);
    if (flock(fd,LOCK_EX)==-1) {
      if (errno==EINTR) continue;
      perror("lockfile: flock()");
      return EXIT_FAILURE;
    }

    close(sync_fd);

    signal(SIGALRM, SIG_IGN);
    while (getppid()!=1) sleep(1);
	   
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
  
