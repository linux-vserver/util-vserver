// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on filetime.cc by Jacques Gelinas
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
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
#include "lib/internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

static void
showHelp(char const *cmd)
{
  WRITE_MSG(1, "Usage:  ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " [--] <filename>\n"
	    "\n"
	    "Shows the relative age of <filename>\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "filetime " VERSION " -- shows age of a file\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main (int argc, char *argv[])
{
  int			idx = 1;
  struct stat		st;
  
  if (argc>=2) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(argv[0]);
    if (strcmp(argv[1], "--version")==0) showVersion();
    if (strcmp(argv[1], "--")       ==0) ++idx;
  }
  if (argc<idx+1)
    WRITE_MSG(2, "No filename specified; use '--help' for more information\n");
  else if (stat(argv[idx], &st)==-1)
    PERROR_Q("stat", argv[idx]);
  else {    
    time_t	now     = time(NULL);
    time_t	since   = now - st.st_mtime;
    int		days    = since / (24*60*60);
    int		today   = since % (24*60*60);
    int		hours   = today / (60*60);
    int		minutes = (today % (60*60)) / 60;

    char	buf[3*sizeof(time_t)*3 + 128];
    size_t	l = 0;

    if (days > 0) {
      l        = utilvserver_fmt_ulong(buf, days);
      buf[l++] = ' ';
      #define MSG	"days, "
      memcpy(buf+l, MSG, sizeof(MSG)-1); l += sizeof(MSG)-1;
    }

    if (hours<10) buf[l++] = '0';
    l += utilvserver_fmt_ulong(buf+l, hours);
    buf[l++] = ':';
    if (minutes<10) buf[l++] = '0';
    l += utilvserver_fmt_ulong(buf+l, minutes);
    buf[l++] = '\n';

    Vwrite(1, buf, l);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

