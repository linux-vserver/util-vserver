// $Id$    --*- c++ -*--

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

#include "compat.h"
#include "vserver.h"
#include "internal.h"
#include "src/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  char		buf[32];
  pid_t		pid;
  
  if (argc==1) pid = vc_X_getinitpid(0);
  else         pid = vc_X_getinitpid(atoi(argv[1]));

  utilvserver_int2str(buf, sizeof buf, pid, 10);

  WRITE_STR(1, buf);
  WRITE_MSG(1, "\n");

  return 0;
}
