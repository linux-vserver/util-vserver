// $Id$    --*- c -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ENSC_WRAPPERS_UNISTD 1
#include "wrappers.h"

int	wrapper_exit_code = 2;

static void
sigHandler(int UNUSED sig)
{
}

int main(int argc, char *argv[])
{
  int		fd;
  char		buf[1];
  bool		res;
  bool		is_root;

  if (argc!=4) {
    WRITE_MSG(2, "Usage: minit-sync <chroot> <filename> <timeout>\n");
    return 1;
  }

  is_root = strcmp(argv[1], "/")==0;

  if (!is_root) {
    Echroot(argv[1]);
    Echdir("/");
  }

  signal(SIGALRM, sigHandler);
  alarm(atoi(argv[3]));
  
  res = ((fd=open(argv[2], O_RDONLY, 0))!=-1 &&
	 read(fd, buf, sizeof buf)!=-1);

  if (fd!=-1) Eclose(fd);
  
  if (!is_root)
    Eunlink(argv[2]);

  return res ? 0 : 1;
}
