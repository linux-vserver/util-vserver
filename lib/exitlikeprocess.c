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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#include <stdio.h>
#include <errno.h>

#include "vserver.h"

static int pid;
static void
signalHandler(int signum)
{
  xid_t xid = vc_get_task_xid(pid);
  if (xid)
    vc_ctx_kill(xid, pid, signum);
  else
    kill(pid, signum);
}

void
vc_exitLikeProcess(int p, int ret)
{
  int status, i;

  pid = p;

  for (i = 0; i < 32; i++)
    signal(i, signalHandler);

retry:
  if (wait4(pid, &status, 0,0)==-1) {
    if (errno==EINTR)
      goto retry;
    perror("wait()");
    exit(ret);
  }

  if (WIFEXITED(status))
    exit(WEXITSTATUS(status));

  if (WIFSIGNALED(status)) {
    struct rlimit	lim = { 0,0 };

    // prevent coredumps which might override the real ones
    setrlimit(RLIMIT_CORE, &lim);
      
    kill(getpid(), WTERMSIG(status));
    exit(1);
  }
  else
    exit(ret);
}
