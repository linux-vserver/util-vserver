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

#include <vserver.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>


#define ENSC_WRAPPERS_PREFIX	"vtag: "
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_TAG			0x4000
#define CMD_CREATE		0x4001
#define CMD_MIGRATE		0x4002
#define CMD_SILENTEXIST		0x4003
#define CMD_SILENT		0x4004


struct option const
CMDLINE_OPTIONS[] = {
  { "help",         no_argument,       0, CMD_HELP },
  { "version",      no_argument,       0, CMD_VERSION },
  { "tag",          required_argument, 0, CMD_TAG },
  { "create",       no_argument,       0, CMD_CREATE },
  { "migrate",      no_argument,       0, CMD_MIGRATE },
  { "silent",       no_argument,       0, CMD_SILENT },
  { "silentexist",  no_argument,       0, CMD_SILENTEXIST },
  { 0,0,0,0 },
};

struct Arguments {
    bool		do_create;
    bool		do_migrate;
    bool		is_silentexist;
    int			verbosity;
    tag_t		tag;
};

int		wrapper_exit_code = 255;

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n    ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " (--create|--migrate) --tag <tag> <opts>* [--] <program> <args>*\n    "
	    "\n"
	    "<opts> can be:\n"
	    "    --silentexist   ...  be silent when context exists already; useful\n"
	    "                         for '--create' only\n"
	    "    --silent        ...  if the feature is not supported, just execute\n"
	    "                         <program>\n"
	    "\n"
	    "'vtag --create' exits with code 254 iff the context exists already.\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vtag " VERSION " -- sets the process's filesystem tag\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2007 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static inline ALWAYSINLINE int
doit(struct Arguments const *args, char *argv[])
{
  tag_t			tag;

  if (!vc_isSupported(vcFEATURE_PPTAG)) {
    if (args->verbosity >= 1) {
      errno = ENOSYS;
      perror(ENSC_WRAPPERS_PREFIX);
      return wrapper_exit_code;
    }
    else
      goto exec;
  }

  if (args->do_create) {
    tag = vc_tag_create(args->tag);
    if (tag==VC_NOCTX) {
      switch (errno) {
	case EEXIST	:
	  if (!args->is_silentexist)
	    perror(ENSC_WRAPPERS_PREFIX "vc_tag_create()");
	  return 254;
	default		:
	  perror(ENSC_WRAPPERS_PREFIX "vc_tag_create()");
	  return wrapper_exit_code;
      }
    }
  }
  else
    tag = args->tag;

  if (args->do_migrate)
    Evc_tag_migrate(tag);

exec:
  execvp(argv[optind], argv+optind);

  PERROR_Q(ENSC_WRAPPERS_PREFIX "execvp", argv[optind]);
  return wrapper_exit_code;
}

int main (int argc, char *argv[])
{
  struct Arguments		args = {
    .tag               = VC_NOCTX,
    .do_create         = false,
    .do_migrate        = false,
    .is_silentexist    = false,
    .verbosity         = 1,
  };

  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_CREATE		:  args.do_create      = true;   break;
      case CMD_MIGRATE		:  args.do_migrate     = true;   break;
      case CMD_SILENTEXIST	:  args.is_silentexist = true;   break;
      case CMD_TAG		:  args.tag = Evc_tagopt2tag(optarg,true); break;
      case CMD_SILENT		:  args.verbosity--;             break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return wrapper_exit_code;
	break;
    }
  }

  if (!args.do_create && !args.do_migrate)
    WRITE_MSG(2, "Neither '--create' nor '--migrate' specified; try '--help' for more information\n");
  else if (args.do_create && args.do_migrate)
    WRITE_MSG(2, "Can not specify '--create' and '--migrate' at the same time; try '--help' for more information\n");
  else if (args.tag==VC_NOCTX)
    WRITE_MSG(2, "You must specify the tag with '--tag'; try '--help' for more information\n");
  else if (optind>=argc)
    WRITE_MSG(2, "No command given; use '--help' for more information.\n");
  else
    return doit(&args, argv);

  return wrapper_exit_code;
}
