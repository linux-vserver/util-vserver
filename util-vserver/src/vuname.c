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

#include "vserver.h"
#include "util.h"

#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <libgen.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_UTSCONTEXT		0x4000
#define CMD_UTSSYSNAME		0x4001
#define CMD_UTSNODENAME		0x4002
#define CMD_UTSRELEASE		0x4003
#define CMD_UTSVERSION		0x4004
#define CMD_UTSMACHINE		0x4005
#define CMD_UTSDOMAINNAME	0x4006


static vc_uts_type const	UTS_MAPPING[7] = {
  vcVHI_CONTEXT, vcVHI_SYSNAME, vcVHI_NODENAME,
  vcVHI_RELEASE, vcVHI_VERSION, vcVHI_MACHINE,
  vcVHI_DOMAINNAME };

struct Arguments {
    bool		handle_opts[DIM_OF(UTS_MAPPING)];
    xid_t		xid;
    bool		do_set;
    char const *	value;
};

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "xid",      required_argument, 0, 'x' },
  { "set",      no_argument,       0, 's' },
  { "get",      no_argument,       0, 'g' },
  { "context",     no_argument, 0, CMD_UTSCONTEXT },
  { "sysname",     no_argument, 0, CMD_UTSSYSNAME },
  { "nodename",    no_argument, 0, CMD_UTSNODENAME },
  { "release",     no_argument, 0, CMD_UTSRELEASE },
  { "version",     no_argument, 0, CMD_UTSVERSION },
  { "machine",     no_argument, 0, CMD_UTSMACHINE },
  { "domainname" , no_argument, 0, CMD_UTSDOMAINNAME },
  { 0,0,0,0 }
};

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [-g] [--xid|x <xid>] [--<TAG>]*\n"
	    "    or  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,	    
	    "  -s  [--xid|x <xid>]  --<TAG> [--] <value>\n\n"
	    " Options:\n"
	    "   -x <xid>  ...  operate on this context (default: current one)\n"
	    "   -g        ...  get and print the value\n"
	    "   -s        ...  set the value\n\n"
	    " Valid TAGs are:\n"
	    "   context, sysname, nodename, release, version, machine, domainname\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vuname " VERSION " -- modifies and shows uname entries of vserver contexts\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static unsigned int
getTrueCount(bool const *field, size_t cnt)
{
  unsigned int	res = 0;
  while (cnt>0) {
    --cnt;
    if (field[cnt]) ++res;
  }

  return res;
}

int main(int argc, char *argv[])
{
  struct Arguments	args = {
    .handle_opts = { 0,0,0,0,0,0,0 },
    .do_set      = false,
    .xid         = VC_SAMECTX,
  };
  unsigned int		opt_cnt;
  size_t		i;
  bool			failed = false;
  bool			passed = false;
  char const *		delim  = "";
  char			result_buf[1024] = { [0] = '\0' };

  assert(DIM_OF(UTS_MAPPING) == DIM_OF(args.handle_opts));
  
  while (1) {
    int		c = getopt_long(argc, argv, "gsx:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    if (c>=CMD_UTSCONTEXT && c<=CMD_UTSDOMAINNAME)
      args.handle_opts[c-CMD_UTSCONTEXT] = true;
    else switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case 'g'		:  args.do_set  = true; break;
      case 's'		:  args.do_set  = true; break;
      case 'x'		:  args.xid     = atoi(optarg); break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  opt_cnt = getTrueCount(args.handle_opts, DIM_OF(args.handle_opts));
  
  if (args.do_set && optind==argc) {
    WRITE_MSG(2, "No value given; use '--help' for more information\n");
    return EXIT_FAILURE;
  }

  if (args.do_set && optind+1>argc) {
    WRITE_MSG(2, "Too much values given; use '--help' for more information\n");
    return EXIT_FAILURE;
  }

  if (args.do_set && opt_cnt<=0) {
    WRITE_MSG(2, "No field given which shall be set; use '--help' for more information\n");
    return EXIT_FAILURE;
  }

  if (args.do_set && opt_cnt>1) {
    WRITE_MSG(2, "Can not set multiple fields; use '--help' for more information\n");
    return EXIT_FAILURE;
  }

  if (!args.do_set && optind!=argc) {
    WRITE_MSG(2, "Can not specifiy a value with '-g'; use '--help' for more information\n");
    return EXIT_FAILURE;
  }

  if (args.do_set) args.value = argv[optind];

  if (!args.do_set && opt_cnt==0)
    for (i=0; i<DIM_OF(args.handle_opts); ++i) args.handle_opts[i]=true;
    
  for (i=0; i<DIM_OF(args.handle_opts); ++i) {
    if (!args.handle_opts[i]) continue;

    if (args.do_set) {
      if (vc_set_vhi_name(args.xid, UTS_MAPPING[i], args.value, strlen(args.value))==-1) {
	perror("vc_set_vhi_name()");
	return EXIT_FAILURE;
      }
    }
    else {
      char		buf[128];
      if (vc_get_vhi_name(args.xid, UTS_MAPPING[i], buf, sizeof(buf)-1)==-1) {
	perror("vc_get_vhi_name()");
	failed = true;
	strcpy(buf, "???");
      }
      else
	passed = true;
      strcat(result_buf, delim);
      strcat(result_buf, buf);
      delim = " ";
    }
  }

  if (!args.do_set && passed) {
    strcat(result_buf, "\n");
    WRITE_STR(1, result_buf);
  }

  return failed ? passed ? 2 : EXIT_FAILURE : EXIT_SUCCESS;
}
