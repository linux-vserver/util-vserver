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

#include "vserver-start.h"
#include <pathconfig.h>

#include <lib_internal/string.h>

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#include <wrappers.h>


inline static bool
checkTTY(char const /*@null@*/ *p)
{
  return p!=0 && access(p, R_OK|W_OK)==0;
}

void
setDefaultTTY(PathInfo const *cfgdir, char const *dflt)
{
  PathInfo	subpath = ENSC_STRING_FIXED("/apps/init/tty");
  char		buf[ENSC_PI_APPSZ(*cfgdir, subpath)];
  char const *	new_tty = 0;

  do {
    PathInfo	ttypath = *cfgdir;
    
    PathInfo_append(&ttypath, &subpath, buf);
    new_tty = String_c_str(&ttypath, buf);
    if (checkTTY(new_tty)) break;

    new_tty = CONFDIR "/.defaults/apps/init/tty";
    if (checkTTY(new_tty)) break;

    new_tty = dflt;
    if (checkTTY(new_tty)) break;

    new_tty = "/dev/null";
  } while (false);

  int		fd_in  = Eopen(new_tty, O_RDONLY, 0);
  if (fd_in!=0) {
    Edup2(fd_in,  0);
    Eclose(fd_in);
  }
  
  int		fd_out = Eopen(new_tty, O_WRONLY, 0600);
  if (fd_out!=1) {
    Edup2(fd_out, 1);
    Eclose(fd_out);
  }
  
  Edup2(1, 2);
}
