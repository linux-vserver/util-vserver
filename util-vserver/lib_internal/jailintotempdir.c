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

#include "jail.h"

#include <stdlib.h>
#include <grp.h>
#include <unistd.h>

bool
jailIntoTempDir()
{
  char		buf[] = "/tmp/jaildir.XXXXXX";
  char const *	d   = mkdtemp(buf);
  gid_t		id = 1;

  if (d==0 ||
      chdir(d)==-1 ||
      rmdir(d)==-1 ||
      chroot(".")==-1 ||
      setgroups(1, &id)==-1 ||
      setgid(id)==-1 ||
      setuid(id)==-1 ||
      getgid()!=id ||
      getuid()!=id)
    return false;

  return true;
}
