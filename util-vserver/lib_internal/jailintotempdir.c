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
jailIntoTempDir(char const **err_pos)
{
  gid_t const	id    = 1;
  char		buf[] = "/tmp/jaildir.XXXXXX";
  char const *	d     = mkdtemp(buf);
  char const *	err   = "mkdtemp()";

  if (d==0 ||
      (err="chdir()",    chdir(d)==-1) ||
      (err="rmdir()",    rmdir(d)==-1) ||
      (err="chroot()",   chroot(".")==-1) ||
      (err="setgroups()",setgroups(1, &id)==-1) ||
      (err="setgid()",   setgid(id)==-1) ||
      (err="setuid()",   setuid(id)==-1) ||
      (err="getgid()",   getgid()!=id) ||
      (err="getuid()",   getuid()!=id)) {
    if (err_pos!=0) *err_pos = err;
    return false;
  }

  return true;
}
