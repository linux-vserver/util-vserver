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

#include "util-lockfile.h"
#include "errinfo.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

static volatile sig_atomic_t	alarm_flag = 0;

static void
alarmFunc(int UNUSED sig)
{
  alarm_flag = 1;
  signal(SIGALRM, alarmFunc);
}

bool
lockfile(int *fd, char const *filename, int op, long timeout,
	 struct ErrorInformation *err)
{
  char const	*errstr = 0;
  void		(*old_sighandler)(int) = 0;

  errstr = "open()";
  *fd = open(filename, O_CREAT|O_RDONLY|O_NOFOLLOW|O_NONBLOCK, 0644);
  if (*fd==-1) goto err;

  if (timeout!=-1) {
    errstr = "siginterrupt()";
    if (siginterrupt(SIGALRM, 1)==-1) goto err;

    errstr = "signal()";
    old_sighandler = signal(SIGALRM, alarmFunc);
    if (old_sighandler==SIG_ERR) goto err;

    alarm(timeout);
  }

  errstr = "flock()";
  while (flock(*fd, op)==-1) {
    if ((errno!=EINTR && errno!=EINTR) || alarm_flag) goto err;
  }

  if (timeout!=-1 && old_sighandler!=0)
    signal(SIGALRM, old_sighandler);

  errstr = "fcntl()";
  if (fcntl(*fd, F_SETFD, FD_CLOEXEC)==-1) goto err;

  return true;

  err:
  if (err) {
    err->pos = errstr;
    err->id  = errno;
  }
  if (timeout!=-1 && old_sighandler!=0)
    signal(SIGALRM, old_sighandler);
  if (*fd!=-1) close(*fd);
  return false;
}
