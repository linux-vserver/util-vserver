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

#include "pathconfig.h"

static void
initModeManually(struct Arguments const UNUSED *args, int argc, char *argv[])
{
  int		i, count=argc/2;
  
  if (argc%2) {
    WRITE_MSG(2, "Odd number of (path,excludelist) arguments\n");
    exit(1);
  }

  if (count<2) {
    WRITE_MSG(2, "No reference path(s) given\n");
    exit(1);
  }

  MatchList_initManually(&global_info.dst_list, 0, strdup(argv[0]), argv[1]);

  --count;
  global_info.src_lists.v = Emalloc(sizeof(struct MatchList) * count);
  global_info.src_lists.l = count;

  for (i=0; i<count; ++i)
    MatchList_initManually(global_info.src_lists.v+i, 0,
			   strdup(argv[2 + i*2]), argv[3 + i*2]);
}


static void
initModeVserver(struct Arguments const UNUSED *args, int argc, char *argv[])
{
  char const				*appdir;
  struct MatchVserverInfo const		dst_vserver = { argv[0], true };
  
  if (argc!=1) {
    WRITE_MSG(2, "More than one vserver is not supported\n");
    exit(1);
  }

  if (!MatchList_initByVserver(&global_info.dst_list, &dst_vserver, &appdir)) {
    WRITE_MSG(2, "unification not configured for this vserver\n");
    exit(1);
  }

  MatchList_initRefserverList(&global_info.src_lists.v,
			      &global_info.src_lists.l,
			      appdir);
  
  free(const_cast(char *)(appdir));
}
