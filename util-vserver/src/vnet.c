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
#include <vserver.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>

#define ENSC_WRAPPERS_PREFIX	"vcontext: "
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_NID			0x4000
#define CMD_CREATE		0x4001
#define CMD_MIGRATE		0x4003
#define CMD_SILENT		0x4007
#define CMD_SILENTEXIST		0x400c

struct option const
CMDLINE_OPTIONS[] = {
  { "help",         no_argument,       0, CMD_HELP },
  { "version",      no_argument,       0, CMD_VERSION },
  { "nid",          required_argument, 0, CMD_NID },
  { "create",       no_argument,       0, CMD_CREATE },
  { "migrate",      required_argument, 0, CMD_MIGRATE },
  { "silent",       no_argument,       0, CMD_SILENT },
  { "silentexist",  no_argument,       0, CMD_SILENTEXIST },
  { 0,0,0,0 }
};
  
struct Arguments {
    bool		do_create;
    bool		do_migrate;
    bool		is_silentexist;
    int			verbosity;
    nid_t		nid;
};

int		wrapper_exit_code = 255;

static void
showHelp(char const *cmd)
{
  WRITE_MSG(1, "Usage:\n    ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " --create  <opts>* [--] <program> <args>*\n    ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " --migrate <opts>* [--] <program> <args>*\n");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " --add|--del <ip>/[<prefix>|<mask>] <opts>* [--] <program> <args>*\n");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " (--flag <flags>*)|(--caps <caps>*) <opts>* [--] <program> <args>*\n"
	    "\n"
	    "<opts> can be:\n"
	    "    --nid <nid>     ...  operate on network context <nid>; in combination\n"
	    "                         with '--create' the network context <nid> instead of\n"
	    "                         a dynamic one will be created, and with '--migrate\n"
	    "                         the context <nid> instead of the current one will\n"
	    "                         be entered\n"
	    "    --silent        ...  be silent\n"
	    "    --silentexist   ...  be silent when context exists already; usefully\n"
	    "                         for '--create' only\n"
	    "\n"
	    "'vnet --create' exits with code 254 iff the context exists already.\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(0);
}


static void
showVersion()
{
  WRITE_MSG(1,
	    "vnet " VERSION " -- manages the creation of network contexts\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static inline ALWAYSINLINE int
doit(struct Arguments const *args, char *argv[])
{
  nid_t		nid;
  
  if (args->do_create) {
    nid = vc_net_create_context(args->nid);
    if (nid==VC_NONID) {
      switch (errno) {
	case EEXIST	:
	  if (!args->is_silentexist)
	    perror(ENSC_WRAPPERS_PREFIX "vc_net_create_context()");
	  return 254;
	default	:
	  perror(ENSC_WRAPPERS_PREFIX "vc_net_create_context()");
	  return wrapper_exit_code;
      }
    }
    tellContext(nid, args->verbosity>=1);
  }
  else
    nid = args->nid;

  
}
  
  
int main (int argc, char *argv[])
{
  struct Arguments		args = {
    .do_create      = false,
    .do_migrate     = false,
    .is_silentexist = false,
    .verbosity      = 1,
    .nid            = VC_DYNAMIC_NID,
  };
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;
    
    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_NID		:  args.nid            = optarg(nid); break;
      case CMD_CREATE		:  args.do_create      = true; break;
      case CMD_MIGRATE		:  args.do_migrate     = true; break;
      case CMD_SILENTEXIST	:  args.is_silentexist = true; break;
      case CMD_SILENT		:  --args.verbosity; break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return 255;
	break;
    }
  }

  if (args.nid==VC_DYNAMIC_NID && args.do_migrate)
    args.nid = Evc_get_task_nid(0);
  
  if (!args.do_create && !args.do_migrate)
    WRITE_MSG(2, "Neither '--create' nor '--migrate specified; try '--help' for more information\n");
  else if (args.do_create  &&  args.do_migrate)
    WRITE_MSG(2, "Can not specify '--create' and '--migrate' at the same time; try '--help' for more information\n");
  else if (!args.do_create && args.nid==VC_DYNAMIC_NID)
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Can not migrate to an unknown context\n");
  else if (optind>=argc)
    WRITE_MSG(2, "No command given; use '--help' for more information.\n");
  else
    return doit(&args, argv);

  return 255;
}
