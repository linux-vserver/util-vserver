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

#include "vserver.h"

#include "src/util.h"
#include <stdlib.h>

static void
checkArgs(int argc, char *argv[])
{
  if (argc==2) {
    if (strcmp(argv[1], "--help")==0) {
      WRITE_MSG(1, "Usage: vserver-info <vserver>\n");
      exit(0);
    }
    if (strcmp(argv[1], "--version")==0) {
      WRITE_MSG(1, "vserver-info " VERSION "\n");
      exit(0);
    }
  }
  else {
    WRITE_MSG(2, "No vserver specified; try '--help' for more inforamtion\n");
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  vcCfgStyle	style = (checkArgs(argc, argv), vc_getVserverCfgStyle(argv[1]));
  char const *	name  = vc_getVserverName(argv[1], style);
  char const *	vdir  = vc_getVserverVdir(argv[1], style, true);

  WRITE_MSG(2, "Style: ");
  switch (style) {
    case vcCFG_NONE		:  WRITE_MSG(2, "CFG_NONE");   break;
    case vcCFG_AUTO		:  WRITE_MSG(2, "CFG_AUTO");   break;
    case vcCFG_LEGACY		:  WRITE_MSG(2, "CFG_LEGACY"); break;
    case vcCFG_RECENT_FULL	:  WRITE_MSG(2, "CFG_RECENT_FULL");  break;
    case vcCFG_RECENT_SHORT	:  WRITE_MSG(2, "CFG_RECENT_SHORT"); break;
    default			:  WRITE_MSG(2, "???"); break;
  }

  WRITE_MSG(2, "\nName:  ");
  if (name==0) WRITE_MSG(2, "<null>");
  else         WRITE_STR(2, name);

  WRITE_MSG(2, "\nVdir:  ");
  if (vdir==0) WRITE_MSG(2, "<null>");
  else         WRITE_STR(2, vdir);

  WRITE_MSG(2, "\n");
  return EXIT_SUCCESS;
}
