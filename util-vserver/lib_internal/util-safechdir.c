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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int
safeChdir(char const *path, struct stat const *exp_stat)
{
  if (strchr(path, '/')!=0) {
    errno = EINVAL;
    return -1;
  }

  {
    struct stat		now_stat;
    if (chdir(path)==-1 ||
	stat(".", &now_stat)==-1) return -1;
    if (exp_stat->st_dev != now_stat.st_dev ||
	exp_stat->st_ino != now_stat.st_ino) {
      // TODO: warning/logging
      errno = EINVAL;
      return -1;
    }
  }

  return 0;
}
