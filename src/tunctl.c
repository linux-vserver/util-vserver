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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

/* Make sure we have the necessary ioctls */
#ifndef TUNSETGROUP
#  define TUNSETGROUP		_IOW('T', 206, int)
#endif
#ifndef TUNSETNID
#  define TUNSETNID		_IOW('T', 215, int)
#endif

#define ENSC_WRAPPERS_PREFIX	"tunctl: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_IOCTL	1
#include <wrappers.h>


#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_NID			0x0001
#define CMD_DEVICE		0x0002
#define CMD_PERSIST		0x0004
#define CMD_NOPERSIST		0x0008
#define CMD_CSUM		0x0010
#define CMD_NOCSUM		0x0020
#define CMD_UID			0x0040
#define CMD_GID			0x0080
#define CMD_LINKTYPE		0x0100
#define CMD_TUN			0x0200
#define CMD_TAP			0x0400
#define CMD_NID_FAILURE_OK	(0x0800 | CMD_NID)

int		wrapper_exit_code  =  255;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",           no_argument,       0, CMD_HELP },
  { "version",        no_argument,       0, CMD_VERSION },
  { "nid",            required_argument, 0, CMD_NID },
  { "device",         required_argument, 0, CMD_DEVICE },
  { "persist",        no_argument,       0, CMD_PERSIST },
  { "~persist",       no_argument,       0, CMD_NOPERSIST },
  { "checksum",       no_argument,       0, CMD_CSUM },
  { "~checksum",      no_argument,       0, CMD_NOCSUM },
  { "uid",            required_argument, 0, CMD_UID },
  { "gid",            required_argument, 0, CMD_GID },
  { "linktype",       required_argument, 0, CMD_LINKTYPE },
  { "tun",            no_argument,       0, CMD_TUN },
  { "tap",            no_argument,       0, CMD_TAP },
  { "nid-failure-ok", required_argument, 0, CMD_NID_FAILURE_OK },
  {0,0,0,0}
};

struct Arguments {
  unsigned int set;
  nid_t nid;
  char *device;
  unsigned persist : 1;
  unsigned checksum : 1;
  uid_t uid;
  gid_t gid;
  int linktype;
  int type;
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage: ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " {--tun|--tap}\n"
	    "    [--persist|--~persist] [--checksum|--~checksum]\n"
	    "    [--nid <nid>] [--uid <uid>] [--gid <gid>] [--linktype <link type>]\n"
	    "    <interface(s)>...\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "tunctl " VERSION " -- controls TUN/TAP devices\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2008 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
doTunctl(struct Arguments *args, char *interface)
{
  int fd;
  struct ifreq ifr;

  fd = EopenD(args->device, O_RDWR, 0);

  strncpy(ifr.ifr_name, interface, IFNAMSIZ);
  ifr.ifr_name[IFNAMSIZ-1] = '\0';
  ifr.ifr_flags = args->type;
  EioctlD(fd, TUNSETIFF, &ifr);

  if (args->set & (CMD_CSUM|CMD_NOCSUM))
    EioctlD(fd, TUNSETNOCSUM, (void *) (long) !args->checksum);
  if (args->set & CMD_UID)
    EioctlD(fd, TUNSETOWNER, (void *) (long) args->uid);
  if (args->set & CMD_GID)
    EioctlD(fd, TUNSETGROUP, (void *) (long) args->gid);
  if (args->set & CMD_NID) {
    if (args->set & CMD_NID_FAILURE_OK)
      ioctl(fd, TUNSETNID, (void *) (long) args->nid);
    else
      EioctlD(fd, TUNSETNID, (void *) (long) args->nid);
  }
  if (args->set & CMD_LINKTYPE)
    EioctlD(fd, TUNSETLINK, (void *) (long) args->linktype);
  if (args->set & (CMD_PERSIST|CMD_NOPERSIST))
    EioctlD(fd, TUNSETPERSIST, (void *) (long) args->persist);

  close(fd);
}

int main(int argc, char *argv[])
{
  struct Arguments args = {
    .set = 0,
    .nid = VC_NOCTX,
    .device = "/dev/net/tun",
    .persist = 0,
    .checksum = 0,
    .uid = -1,
    .gid = -1,
    .linktype = -1,
    .type = IFF_TUN,
  };
  char **interface;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    signed long tmp = 0;
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();

      case CMD_NID_FAILURE_OK:
      case CMD_NID	:  args.nid = Evc_nidopt2nid(optarg, true);	break;
      case CMD_DEVICE	:  args.device = optarg;			break;
      case CMD_PERSIST	:  args.persist = 1;				break;
      case CMD_NOPERSIST:  args.persist = 0;				break;
      case CMD_CSUM	:  args.checksum = 1;				break;
      case CMD_NOCSUM	:  args.checksum = 0;				break;
      case CMD_TUN	:  args.type = IFF_TUN;				break;
      case CMD_TAP	:  args.type = IFF_TAP;				break;
      case CMD_UID	:
	if (!isNumber(optarg, &tmp, true)) {
	  WRITE_MSG(2, "Uid '");
	  WRITE_STR(2, optarg);
	  WRITE_MSG(2, "' is not an integer.\n");
	}
	args.uid = (uid_t) tmp;
	break;
      case CMD_GID	:
	if (!isNumber(optarg, &tmp, true)) {
	  WRITE_MSG(2, "Gid '");
	  WRITE_STR(2, optarg);
	  WRITE_MSG(2, "' is not an integer.\n");
	}
	args.gid = (gid_t) tmp;
	break;
      case CMD_LINKTYPE	:
	if (!isNumber(optarg, &tmp, true)) {
	  WRITE_MSG(2, "Link-type '");
	  WRITE_STR(2, optarg);
	  WRITE_STR(2, "' is not an integer.\n");
	}
	args.linktype = (int) tmp;
	break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return 255;
	break;
    }

    args.set |= c;
  }

  for (interface = argv + optind; *interface; interface++) {
    doTunctl(&args, *interface);
  }

  return EXIT_SUCCESS;
}
