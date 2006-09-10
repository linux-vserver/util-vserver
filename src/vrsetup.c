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

#include "util.h"

#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_IOCTL	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

int wrapper_exit_code = 1;


#define VROOT_SET_DEV           0x5600
#define VROOT_CLR_DEV           0x5601
#define VROOT_INC_USE           0x56FE
#define VROOT_DEC_USE           0x56FF

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, 'h' },
  { "version",  no_argument,  0, 'v' },
  { 0,0,0,0 }
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-dID] <rootdev>\n"
	    "    or  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " <rootdev> <real-rootdev>\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vrsetup " VERSION " -- set up and control vroot devices\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}


int main(int argc, char *argv[])
{
  bool		do_delete    = false;
  bool		do_decrement = false;
  bool		do_increment = false;
  bool		do_setup     = false;
  char const *	root_device      = 0;
  char const *	real_root_device = 0;
  int		fd;
  
  while (1) {
    int		c = getopt_long(argc, argv, "dID", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case 'v'		:  showVersion();
      case 'd'		:  do_delete    = true; break;
      case 'D'		:  do_decrement = true; break;
      case 'I'		:  do_increment = true; break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  do_setup = !(do_delete || do_decrement || do_increment);

  if (optind+1>argc) {
    WRITE_MSG(2, "No vroot-device given\n");
    return EXIT_FAILURE;
  }
  
  if (do_setup && optind+2>argc) {
    WRITE_MSG(2, "No real root-device given\n");
    return EXIT_FAILURE;
  }
	
  root_device = argv[optind];
  if (do_setup) real_root_device = argv[optind+1];

  fd = EopenD(root_device, O_RDONLY, 0);
  if      (do_increment) Eioctl(fd, VROOT_INC_USE, 0);
  else if (do_decrement) Eioctl(fd, VROOT_DEC_USE, 0);
  else if (do_delete)    Eioctl(fd, VROOT_CLR_DEV, 0);
  else {
    int		dfd = EopenD(real_root_device, O_RDONLY, 0);
    Eioctl(fd, VROOT_SET_DEV, (void*)dfd);
    Eclose(dfd);
  }

  Eclose(fd);
  return EXIT_SUCCESS;
}
