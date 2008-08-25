// $Id$    --*- c -*--

// Copyright (C) 2007 Daniel Hokka Zakrisson
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
#include <lib/internal.h>

#include <vserver.h>

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define ENSC_WRAPPERS_PREFIX	"vsysctl: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_DIRENT	1
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_IO	1
#include <wrappers.h>


#define PROC_SYS_DIRECTORY	"/proc/sys"


#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_XID			0x4000
#define CMD_DIR			0x4001
#define CMD_MISSINGOK		0x4002

int		wrapper_exit_code  =  1;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,       0, CMD_HELP },
  { "version",    no_argument,       0, CMD_VERSION },
  { "xid",        required_argument, 0, CMD_XID },
  { "dir",        required_argument, 0, CMD_DIR },
  { "missingok",  no_argument,       0, CMD_MISSINGOK },
  {0,0,0,0}
};

static void
showHelp(int fd, char const *cmd)
{
  WRITE_MSG(fd, "Usage: ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --xid <xid> --dir <directory> [--missingok] -- <command> <args>*\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vsysctl " VERSION " -- sets sysctl values during guest boot\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2007 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

void handle_setting(const char *dir, const char *name)
{
  int	len_dir = strlen(dir), len_name = strlen(name);
  char	filename[len_dir+1+len_name+sizeof("/setting")];
  char	setting[128], value[128], *ptr;
  int	fd;
  size_t setting_len, value_len;

  strcpy(filename, dir);
  *(filename+len_dir) = '/';
  strcpy(filename+len_dir+1, name);

#define READFILE(f) \
  strcpy(filename+len_dir+1+len_name, "/" #f); \
  fd = EopenD(filename, O_RDONLY, 0); \
  f##_len = Eread(fd, f, sizeof(f)); \
  if (f##_len == sizeof(f)) { \
    errno = EOVERFLOW; \
    perror(ENSC_WRAPPERS_PREFIX "read"); \
    exit(EXIT_FAILURE); \
  } \
  f[f##_len] = '\0'; \
  Eclose(fd);

  READFILE(setting);
  READFILE(value);

  /* replace all . with / in setting to get a filename */
  for (ptr = strchr(setting, '.'); ptr; ptr = strchr(ptr, '.'))
    *ptr = '/';

  /* we just want the first line, and not the linefeed */
  if ((ptr = strchr(setting, '\n')) != NULL)
    *ptr = '\0';

  fd = EopenD(setting, O_WRONLY, 0);
  EwriteAll(fd, value, value_len);
  Eclose(fd);
}

int main(int argc, char *argv[])
{
  xid_t		xid	= VC_NOCTX;
  const char	*dir	= NULL;
  bool		missing	= false;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0]);
      case CMD_VERSION	:  showVersion();
      case CMD_XID	:  xid = Evc_xidopt2xid(optarg, true);	break;
      case CMD_DIR	:  dir = optarg;			break;
      case CMD_MISSINGOK:  missing = true;			break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (dir != NULL) {
    int		  curdir = EopenD(".", O_RDONLY, 0);
    DIR		  *dp;
    struct dirent *de;

    if (chdir(PROC_SYS_DIRECTORY) == -1)
      goto exec;

    dp = opendir(dir);
    if (dp != NULL) {
      while ((de = Ereaddir(dp)) != NULL) {
	if (*de->d_name == '.')
	  continue;
	handle_setting(dir, de->d_name);
      }
      Eclosedir(dp);
    }
    else if (!missing) {
      perror(ENSC_WRAPPERS_PREFIX "opendir");
      exit(wrapper_exit_code);
    }

    Efchdir(curdir);
  }

exec:
  Eexecvp(argv[optind], argv+optind);
  return EXIT_FAILURE;
}
