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


#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

int wrapper_exit_code = 1;

int main(int argc, char *argv[])
{
  char		buf[1000];

  while (true) {
    char	*ptr = buf;
    char	c;
    do {
      if (read(0, &c, 1)==0) break;
      if (c=='\n') break;
      *ptr++ = c;
    } while (ptr<buf+sizeof(buf));
    *ptr = '\0';
    if (ptr==buf) break;

    switch (buf[0]) {
      case 'P'		: {
	struct passwd	*pw;
	
	pw = getpwnam(buf+1);
	printf("P(%s) = ", buf+1);
	if (pw) printf("%u\n", pw->pw_uid);
	else    printf("(null)\n");
	
	break;
      }

      case 'G'		: {
	struct group	*gr;
	
	gr = getgrnam(buf+1);
	printf("G(%s) = ", buf+1);
	if (gr) printf("%u\n", gr->gr_gid);
	else    printf("(null)\n");
	
	break;
      }

      case 'C'		:
	switch (buf[1]) {
	  case 'g'	:  endgrent(); break;
	  case 'p'	:  endpwent(); break;
	  default	:  abort(); break;
	}
	break;

      default		:
	abort();
    }
  }

  {
    char const *	cmd[] = { "/bin/grep", "^s_context", "/proc/self/status", 0 };
    Eexecv(cmd[0], cmd);
  }
}
