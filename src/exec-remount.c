// $Id$    --*- c -*--

// Copyright (C) 2008 Daniel Hokka Zakrisson
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

#include <vserver.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <errno.h>

#define ENSC_WRAPPERS_PREFIX	"exec-remount: "
#define ENSC_WRAPPERS_MOUNT	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_MTAB		0x2001

#ifndef MNT_DETACH
#define MNT_DETACH		0x0002
#endif

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",	no_argument,       0, CMD_HELP },
  { "version",	no_argument,       0, CMD_VERSION },
  { "mtab",	required_argument, 0, CMD_MTAB },
  { NULL, 0, 0, 0 }
};

int		wrapper_exit_code  =  255;

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n    ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd, " [--mtab <file>] <mount points>* -- <command> <args>*\n");
  WRITE_MSG(fd, "\n"
		"Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion(void)
{
  WRITE_MSG(1,
	"exec-remount " VERSION " -- remounts specified mount points and executes a program\n"
	"This program is part of " PACKAGE_STRING "\n\n"
	"Copyright (c) 2008 Daniel Hokka Zakrisson\n"
	VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
do_remount(char const *mount)
{
  /* FIXME: Read this from mtab */
  if (strcmp(mount, "/proc") == 0)
    Emount("proc", "proc", "proc", 0, NULL);
  else if (strcmp(mount, "/sys") == 0)
    Emount("sysfs", "sys", "sysfs", 0, NULL);
  else {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "unknown mount point: ");
    WRITE_STR(2, mount);
    WRITE_MSG(2, "\n");
  }
}

int main(int argc, char *argv[])
{
  int i;
  char const *mtab = "/etc/mtab";

  while (1) {
    int c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_MTAB	:  mtab = optarg; break;
      default		:  showHelp(2, argv[0], 1);
    }
  }

  for (i = optind; argv[i] != NULL && strcmp(argv[i], "--") != 0; i++) {
    if (vc_isSupported(vcFEATURE_PIDSPACE)) {
      /* + 1 to strip the leading / */
      if (umount2(argv[i] + 1, MNT_DETACH) == 0)
	do_remount(argv[i]);
    }
  }

  if (argv[i] == NULL)
    showHelp(2, argv[0], 1);

  i++;
  EexecvpD(argv[i], argv+i);
  /* NOTREACHED */
  return 1;
}
