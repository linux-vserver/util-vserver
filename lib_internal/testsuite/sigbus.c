// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_SOCKET	1
#include <wrappers.h>

int wrapper_exit_code = 1;

#define TEST_BLOCKSIZE	0x20000
static bool			is_gremlin = false;
static int			sync_p[2];

static void
testit()
{
  if (!is_gremlin) return;

  char	c;

  Esend(sync_p[1], ".", 1, 0);
  Erecv(sync_p[1], &c,  1, 0);
}

#define TESTSUITE_COPY_CODE	testit()

#include "../unify.h"
#include "../unify-copy.c"
#include "../unify-settime.c"

static bool
checkTrunc(char const *src,
	   char const *dst,
	   struct stat const *st,
	   size_t pos)
{
  pid_t		pid = Efork();
  
  if (pid==0) {
    char		c;
    
    Erecv(sync_p[0], &c, 1, 0);
    Etruncate(src, pos);
    Esend(sync_p[0], &c, 1, 0);
    exit(0);
  }

  unlink(dst);
  return !copyReg(src, st, dst);
}

int main()
{
  char		f_name0[] = "/tmp/sigbus.XXXXXX";
  char		f_name1[] = "/tmp/sigbus.XXXXXX";
  int		fd_src    = mkstemp(f_name0);
  int		fd_dst    = mkstemp(f_name1);
  char		buf[TEST_BLOCKSIZE] = { [0] = '\0' };
  struct stat	st;
  bool		res;
  
  fd_src = 
  
  write(fd_src, buf, sizeof(buf));
  close(fd_src);
  close(fd_dst);

  unlink(f_name1);
  stat(f_name0, &st);
  if (!copyReg(f_name0, &st, f_name1))
    return EXIT_FAILURE;


  is_gremlin = true;

  Esocketpair(AF_LOCAL, SOCK_STREAM, 0, sync_p);
  signal(SIGCHLD, SIG_IGN);

  res = (checkTrunc(f_name0, f_name1, &st, TEST_BLOCKSIZE/2) &&
	 checkTrunc(f_name0, f_name1, &st, 0x2345));

  unlink(f_name0);
  unlink(f_name1);
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}
