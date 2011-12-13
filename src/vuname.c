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
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <strings.h>

#define ENSC_WRAPPERS_PREFIX	"vuname: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_IO	1
#define ENSC_WRAPPERS_VSERVER	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_DIR			0x4007
#define CMD_MISSINGOK		0x4008

int			wrapper_exit_code = 255;

static vc_uts_type const	UTS_MAPPING[7] = {
  vcVHI_CONTEXT, vcVHI_SYSNAME, vcVHI_NODENAME,
  vcVHI_RELEASE, vcVHI_VERSION, vcVHI_MACHINE,
  vcVHI_DOMAINNAME };

#define DECL(UTS) [vcVHI_ ## UTS] = #UTS
static char const * const	UTS_STRINGS[] = {
  DECL(CONTEXT), DECL(SYSNAME), DECL(NODENAME),
  DECL(RELEASE), DECL(VERSION), DECL(MACHINE),
  DECL(DOMAINNAME)
};

struct Tag {
    bool		is_set;
    char const *	value;
};

struct Arguments {
    struct Tag		tags[DIM_OF(UTS_MAPPING)];
    xid_t		xid;
    bool		do_set;
    char const *	dir;
    bool		is_missingok;
};

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",        no_argument,       0, CMD_HELP },
  { "version",     no_argument,       0, CMD_VERSION },
  { "xid",         required_argument, 0, 'x' },
  { "set",         no_argument,       0, 's' },
  { "get",         no_argument,       0, 'g' },
  { "dir",         required_argument, 0, CMD_DIR },
  { "missingok",   no_argument,       0, CMD_MISSINGOK },
  { 0,0,0,0 }
};

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    "  [-g] --xid <xid> <TAG>*\n"
	    "    or  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    "  -s --xid <xid> -t <TAG>=<VALUE> [--] [<command> <args>*]\n"
	    "    or  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,	    
	    "  --dir <dir> --xid <xid> [--missingok] [--] [<command> <args>*]\n\n"
	    " Options:\n"
	    "   -g           ...  get and print the value\n"
	    "   -s           ...  set the value\n\n"
	    "   --xid <xid>  ...  operate on this context; 'self' means the current one\n"
	    "   -t <TAG>=<VALUE>\n"
	    "                ...  set <TAG> to <VALUE>; this option can be repeated multiple time\n"
	    "   --dir <dir>  ...  read values from files in <dir>. These files must\n"
	    "                     have a valid TAG as their name\n"
	    "   --missingok  ...  do not fail when the <DIR> from '--dir' does not exist.\n"
	    "\n"
	    " Possible values for TAG are:\n"
	    "   context, sysname, nodename, release, version, machine, domainname\n"
	    "\n"
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

static void
setFromDir(char const *pathname, bool is_missingok, xid_t xid)
{
  struct stat		st;
  size_t		i;
  size_t		l_pathname = strlen(pathname);
  char			buf[l_pathname + sizeof("/domainname") + 32];
  
  if (stat(pathname, &st)==-1) {
    if (errno==ENOENT && is_missingok) return;
    PERROR_Q(ENSC_WRAPPERS_PREFIX "fstat", pathname);
    exit(wrapper_exit_code);
  }

  memcpy(buf, pathname, l_pathname);
  if (l_pathname>0 && pathname[l_pathname-1]!='/')
    buf[l_pathname++] = '/';
  
  for (i=0; i<DIM_OF(UTS_STRINGS); ++i) {
    char *	ptr   = buf+l_pathname;
    int		fd;

    // ignore unimplemented uts-names
    if (UTS_STRINGS[i]==0) continue;
    strcpy(ptr, UTS_STRINGS[i]);
    for (;*ptr;++ptr) *ptr = tolower(*ptr);
    fd = open(buf, O_RDONLY);
    if (fd!=-1) {
      size_t	l = Elseek(fd, 0, SEEK_END);
      char	name[l+1];
      Elseek(fd,0,SEEK_SET);
      EreadAll(fd, name, l);
      while (l>0 && name[l-1]=='\n') --l;
      name[l] = '\0';
      Eclose(fd);

      if (vc_set_vhi_name(xid, (vc_uts_type)(i), name, l)==-1) {
	PERROR_U(ENSC_WRAPPERS_PREFIX "vc_set_vhi_name", UTS_STRINGS[i]);
	exit(wrapper_exit_code);
      }
    }
  }
}

static size_t
findUtsIdx(char const *str, size_t len)
{
  size_t		i;
  for (i=0; i<DIM_OF(UTS_STRINGS); ++i)
    if (UTS_STRINGS[i]!=0 && strncasecmp(UTS_STRINGS[i], str, len)==0)
      return i;

  WRITE_MSG(2, "Tag '");
  Vwrite   (2, str, len);
  WRITE_STR(2, "' not recognized\n");
  exit(wrapper_exit_code);
}

static void
registerValue(char const *str, struct Tag tags[DIM_OF(UTS_MAPPING)])
{
  char const *	ptr = strchr(str, '=');
  size_t	idx;
  
  if (ptr==0) ptr = str + strlen(str);
  assert(*ptr=='=' || *ptr=='\0');

  idx = findUtsIdx(str, ptr-str);

  if (*ptr=='=') ++ptr;
  tags[idx].is_set = true;
  tags[idx].value  = ptr;
}

static void
printUtsValue(xid_t xid, int val)
{
  char	buf[128];
  if (vc_get_vhi_name(xid, val, buf, sizeof(buf)-1)==-1)
    WRITE_MSG(1, "???");
  else
    WRITE_STR(1, buf);
  
}

int main(int argc, char *argv[])
{
  struct Arguments	args = {
    .tags        = { [0] = {false,0} },
    .do_set      = false,
    .dir         = 0,
    .is_missingok= false,
    .xid         = VC_NOCTX,
  };
  size_t		i;

  assert(DIM_OF(UTS_MAPPING) == DIM_OF(args.tags));
  
  while (1) {
    int		c = getopt_long(argc, argv, "+gst:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_DIR	:  args.dir          = optarg; break;
      case CMD_MISSINGOK:  args.is_missingok = true;   break;
      case 'g'		:  args.do_set       = false;  break;
      case 's'		:  args.do_set       = true;   break;
      case 'x'		:  args.xid = Evc_xidopt2xid(optarg,true); break;
      case 't'		:  registerValue(optarg, args.tags);       break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (args.xid==VC_NOCTX) {
    WRITE_MSG(2, "No context specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  if (args.dir)
    setFromDir(args.dir, args.is_missingok, args.xid);
  else if (args.do_set) {
    for (i=0; i<DIM_OF(args.tags); ++i) {
      if (!args.tags[i].is_set) continue;
      Evc_set_vhi_name(args.xid, i, args.tags[i].value, strlen(args.tags[i].value));
    }
  }
  else if (optind==argc) {
    char const *	delim = "";
    for (i=0; i<DIM_OF(UTS_MAPPING); ++i) {
      WRITE_STR(1, delim);
      printUtsValue(args.xid, i);
      delim = " ";
    }
    WRITE_MSG(1, "\n");

    return EXIT_SUCCESS;
  }
  else {
    char const *	delim = "";
    while (optind <argc) {
      int		idx = findUtsIdx(argv[optind], strlen(argv[optind]));
      WRITE_STR(1, delim);
      printUtsValue(args.xid, idx);
      delim = " ";

      ++optind;
    }
    WRITE_MSG(1, "\n");
    
    return EXIT_SUCCESS;
  }

  if (optind<argc)
    EexecvpD(argv[optind], argv+optind);

  return EXIT_SUCCESS;
}
