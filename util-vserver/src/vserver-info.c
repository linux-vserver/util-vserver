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

#include "util.h"
#include "wrappers.h"

#include "internal.h"
#include "vserver.h"

#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <stdbool.h>

typedef enum { tgNONE,tgCONTEXT, tgRUNNING,
	       tgVDIR, tgNAME, tgCFGDIR, tgAPPDIR }	VserverTag;

static struct {
    char const * const	tag;
    VserverTag const	val;
    char const * const	descr;
}  const TAGS[] = {
  { "CONTEXT", tgCONTEXT, "the current and/or assigned context" },
  { "RUNNING", tgRUNNING, "gives out '1' when vserver is running; else, it fails without output" },
  { "VDIR",    tgVDIR,    "gives out the root-directory of the vserver" },
  { "NAME",    tgNAME,    "gives out the name of the vserver" },
  { "CFGDIR",  tgCFGDIR,  "gives out the configuration directory of the vserver" },
  { "APPDIR",  tgAPPDIR,  "gives out the name of the toplevel application cfgdir" },
};

#define TAGS_COUNT	(sizeof(TAGS)/sizeof(TAGS[0]))

int wrapper_exit_code = 1;

static struct option const
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
	    " [-q] <vserver> <tag>\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vserver-info " VERSION " -- returns information about vservers\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
showTags()
{
  char const *		delim = "";
  size_t	i;

  WRITE_MSG(1, "Valid tags are: ");
  for (i=0; i<TAGS_COUNT; ++i) {
    WRITE_STR(1, delim);
    WRITE_STR(1, TAGS[i].tag);

    delim = ", ";
  }
  WRITE_MSG(1, "\n");
  exit(0);
}

static VserverTag
stringToTag(char const *str)
{
  size_t	i;
  for (i=0; i<TAGS_COUNT; ++i)
    if (strcmp(TAGS[i].tag, str)==0) return TAGS[i].val;

  return tgNONE;
}

static int
execQuery(char const *vserver, VserverTag tag, int argc, char *argv[])
{
  char const *		res = 0;
  char 			buf[sizeof(xid_t)*4 + 16];
  xid_t			ctx;
  
  switch (tag) {
    case tgNAME		:  res = vc_getVserverName(vserver, vcCFG_AUTO); break;
    case tgVDIR		:
      res = vc_getVserverVdir(vserver, vcCFG_AUTO, argc>0 && atoi(argv[0]));
      break;
    case tgCFGDIR	:  res = vc_getVserverCfgDir(vserver, vcCFG_AUTO);     break;
    case tgAPPDIR	:
      res = vc_getVserverAppDir(vserver, vcCFG_AUTO, argc==0 ? "" : argv[0]);
      break;
    case tgCONTEXT	:
      ctx = vc_getVserverCtx(vserver, vcCFG_AUTO, true, 0);
      if (ctx!=VC_NOCTX) {
	utilvserver_fmt_long(buf, ctx);
	res = buf;
      }
      break;
      
    case tgRUNNING	:
      res = (vc_getVserverCtx(vserver, vcCFG_AUTO, false, 0)==VC_NOCTX) ? 0 : "1";
      break;

    default		:  assert(false); abort();  // TODO
  }

  if (res==0) return EXIT_FAILURE;
  WRITE_STR(1, res);
  WRITE_MSG(1, "\n");
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  bool		quiet = false;
  char const *	vserver;
  VserverTag	tag;
  
  while (1) {
    int		c = getopt_long(argc, argv, "ql", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case 'v'		:  showVersion();
      case 'l'		:  showTags();
      case 'q'		:  quiet = true; break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	exit(1);
	break;
    }
  }

  if (optind+2>argc) {
    WRITE_MSG(2, "No vserver or tag give; please try '--help' for more information.\n");
    exit(1);
  }

  vserver = argv[optind];
  tag     = stringToTag(argv[optind+1]);

  if (tag==tgNONE) {
    WRITE_MSG(2, "Unknown tag; use '-l' to get list of valid tags\n");
    exit(1);
  }

  if (quiet) {
    int		fd = Eopen("/dev/null", O_WRONLY, 0644);
    Edup2(fd, 1);
    Eclose(fd);
  }

  return execQuery(vserver, tag, argc-(optind+2), argv+optind+2);
}
