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

#include "command.h"
#include "util.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

static inline bool
initPipes(int p[2])
{
  return (pipe(p)!=-1 &&
	  fcntl(p[1], F_SETFD, FD_CLOEXEC)!=-1);
}

bool
Command_exec(struct Command *cmd, bool do_fork)
{
  int		p[2];

  if (cmd->params_style_==parVEC)
    Vector_zeroEnd(&cmd->params.v);

  if (!do_fork)
    cmd->pid = 0;
  else if (!initPipes(p) ||
	   (cmd->pid = fork())==-1) {
    cmd->err = errno;
    return false;
  }
  
  if (cmd->pid==0) {
    char const **	argv = { 0 };
    
    if (do_fork) close(p[0]);

    switch (cmd->params_style_) {
      case parVEC	:  argv = cmd->params.v.data; break;
      case parDATA	:  argv = cmd->params.d;      break;
      default		:  break;
    }

    execv(cmd->filename ? cmd->filename : argv[0],
	  reinterpret_cast(char **const)(argv));
    cmd->err = errno;
    assert(cmd->err != 0);

    if (do_fork) {
      write(p[1], &cmd->err, sizeof(cmd->err));
      _exit(1);	// implicates 'close(p[1])'
    }
  }
  else {
    close(p[1]);
    if (read(p[0], &cmd->err, sizeof(cmd->err))==0)
      cmd->err = 0;
    else	// cleanup zombies
      while (wait4(cmd->pid, 0,0,0)==-1 &&
	     (errno==EINTR || errno==EAGAIN)) {};
    close(p[0]);
  }

  return cmd->err==0;
}
