// $Id$    --*- c -*--

// Copyright (C) 2006 Daniel Hokka Zakrisson
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

#include <getopt.h>
#include <errno.h>

#define ENSC_WRAPPERS_PREFIX	"vdevmap: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001

int		wrapper_exit_code  =  1;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,       0, CMD_HELP },
  { "version",    no_argument,       0, CMD_VERSION },
  { "xid",        required_argument, 0, 'x' },
  { "set",        no_argument,       0, 's' },
  { "unset",      no_argument,       0, 'u' },
  { "open",       no_argument,       0, 'o' },
  { "create",     no_argument,       0, 'c' },
  { "remap",      no_argument,       0, 'r' },
  { "flags",      required_argument, 0, 'f' },
  { "device",     required_argument, 0, 'd' },
  { "target",     required_argument, 0, 't' },
  {0,0,0,0}
};

static void
showHelp(int fd, char const *cmd)
{
  WRITE_MSG(fd, "Usage: ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --xid <xid> {--set|--unset} [--flags <flags>] [--open] [--create]\n"
	    "        [--device <dev>] [--remap --target <dev>]\n"
	    "\n"
	    "    --flags <flags>     Set the specified flags\n"
	    "    --open              Allow opening of the device\n"
	    "    --create            If SECURE_MKNOD is given, allow mknod(2)\n"
	    "    --device <dev>      Device to apply the command to\n"
	    "    --remap             Remap the device to the target\n"
	    "    --target <dev>      Target for --remap\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vdevmap " VERSION " -- manages device mappings\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2006 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main(int argc, char *argv[])
{
  xid_t		xid		= VC_NOCTX;
  bool		allow_open	= false;
  bool		allow_create	= false;
  bool		do_remap	= false;
  uint32_t	flags		= 0;
  char		*device		= NULL;
  char		*target		= NULL;
  bool		set		= true;
  unsigned long	tmp		= 0;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+x:suocrf:d:t:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0]);
      case CMD_VERSION	:  showVersion();
      case 'x'		:  xid = Evc_xidopt2xid(optarg, true);	break;
      case 'o'		:  allow_open = true;			break;
      case 'c'		:  allow_create = true;			break;
      case 'r'		:  do_remap = true;			break;
      case 'd'		:  device = optarg;			break;
      case 't'		:  target = optarg;			break;
      case 's'		:  set = 1;				break;
      case 'u'		:  set = 0;				break;
      case 'f'		:
	if (!isNumberUnsigned(optarg, &tmp, false)) {
	  WRITE_MSG(2, "Invalid flags argument: '");
	  WRITE_STR(2, optarg);
	  WRITE_MSG(2, "'; try '--help' for more information\n");
	  return EXIT_FAILURE;
	}
	flags |= (uint32_t) tmp;
	break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (allow_open)	flags |= VC_DATTR_OPEN;
  if (allow_create)	flags |= VC_DATTR_CREATE;
  if (do_remap)		flags |= VC_DATTR_REMAP;

  if (target && !do_remap)
    WRITE_MSG(2, "Target specified without --remap; try '--help' for more information\n");
  else if (xid==VC_NOCTX)
    WRITE_MSG(2, "No xid specified; try '--help' for more information\n");
  else if (optind!=argc)
    WRITE_MSG(2, "Unused argument(s); try '--help' for more information\n");
  else if (set && vc_set_mapping(xid, device, target, flags)==-1)
      perror("vc_set_mapping()");
  else if (!set && vc_unset_mapping(xid, device, target, flags)==-1)
      perror("vc_unset_mapping()");
  else
    return EXIT_SUCCESS;

  return EXIT_FAILURE;
}
