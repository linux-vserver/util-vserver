// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#include <libgen.h>
#include <errno.h>
#include <signal.h>
#include <sched.h>

#define ENSC_WRAPPERS_PREFIX	"vdlimit: "
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
  { "set",        required_argument, 0, 's' },
  { "remove",     no_argument,       0, 'd' },
  { "flags",      required_argument, 0, 'f' },
  {0,0,0,0}
};

static void
showHelp(int fd, char const *cmd)
{
  WRITE_MSG(fd, "Usage: ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --xid <xid> [--flags <flags>] (--set <limit>=<value>|--remove) <mount point>\n"
	    "\n"
	    "    --set|-s <limit>=<value>  ...  set <limit> to <value>, where limit is \n"
	    "                           one of: space_used, space_total, inodes_used,\n"
	    "                           inodes_total, reserved\n"
	    "    --remove|-d       ...  removes the disk limit for <xid> from <mount point>\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vdlimit " VERSION " -- manages disk limits\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2005 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
setDlimit(char const *filename, xid_t xid, uint32_t flags, struct vc_ctx_dlimit const *limit)
{
  bool		was_added = false;

  if (vc_get_dlimit(filename, xid, flags, 0) == -1) {
    if (vc_add_dlimit(filename, xid, flags) == -1) {
      perror(ENSC_WRAPPERS_PREFIX "vc_add_dlimit()");
      exit(wrapper_exit_code);
    }

    was_added = true;
  }

  if (vc_set_dlimit(filename, xid, flags, limit) == -1) {
    perror(ENSC_WRAPPERS_PREFIX "vc_set_dlimit()");

    if (was_added &&
	vc_rem_dlimit(filename, xid, flags)==-1)
      perror(ENSC_WRAPPERS_PREFIX "vc_rem_dlimit()");

    exit(wrapper_exit_code);
  }
}

static void
remDlimit(char const *filename, xid_t xid, uint32_t flags)
{
  if (vc_rem_dlimit(filename, xid, flags) == -1) {
    perror(ENSC_WRAPPERS_PREFIX "vc_rem_dlimit()");
    exit(wrapper_exit_code);
  }
}

static void
writeInt(int fd, char const *prefix, unsigned int val)
{
  char		buf[sizeof(val)*3 + 2];
  size_t	len = utilvserver_fmt_uint(buf, val);

  if (prefix)
    WRITE_STR(fd, prefix);
  Vwrite(fd, buf, len);
}

static void
printDlimit(char const *filename, xid_t xid, uint32_t flags, bool formatted)
{
  struct vc_ctx_dlimit		limit;
  
  if (vc_get_dlimit(filename, xid, flags, &limit) == -1) {
    perror(ENSC_WRAPPERS_PREFIX "vc_get_dlimit()");
    exit(wrapper_exit_code);
  }

  if (formatted) {
    writeInt (1, 0, xid);
    WRITE_MSG(1, " ");
    WRITE_STR(1, filename);
    writeInt (1, "\nspace_used=",   limit.space_used);
    writeInt (1, "\nspace_total=",  limit.space_total);
    writeInt (1, "\ninodes_used=",  limit.inodes_used);
    writeInt (1, "\ninodes_total=", limit.inodes_total);
    writeInt (1, "\nreserved=",     limit.reserved);
    WRITE_MSG(1, "\n");
  }
  else {
    writeInt (1, 0,   xid);
    writeInt (1, " ", limit.space_used);
    writeInt (1, " ", limit.space_total);
    writeInt (1, " ", limit.inodes_used);
    writeInt (1, " ", limit.inodes_total);
    writeInt (1, " ", limit.reserved);
    WRITE_MSG(1, " ");
    WRITE_STR(1, filename);
    WRITE_MSG(1, "\n");
  }
}


static bool
setDLimitField(struct vc_ctx_dlimit *dst, char const *opt)
{
  uint_least32_t	*ptr;
  char const * const	orig_opt = opt;

#define GET_VAL_PTR(CMP, VAL)						\
  (strncmp(opt, CMP "=", sizeof(CMP))==0) ?				\
  (opt+=sizeof(CMP), &VAL) : 0
  
  if      ((ptr=GET_VAL_PTR("space_used",   dst->space_used))!=0)   {}
  else if ((ptr=GET_VAL_PTR("space_total",  dst->space_total))!=0)  {}
  else if ((ptr=GET_VAL_PTR("inodes_used",  dst->inodes_used))!=0)  {}
  else if ((ptr=GET_VAL_PTR("inodes_total", dst->inodes_total))!=0) {}
  else if ((ptr=GET_VAL_PTR("reserved",     dst->reserved))!=0)     {}
  else      ptr=0;

#undef  GET_VAL_PTR  

  if (ptr!=0 && *ptr==VC_CDLIM_KEEP) {
    char	*endptr;
    long	val = strtol(opt, &endptr, 0);

    if (*opt==0 || *endptr!='\0') {
      WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "can not parse number in '");
      WRITE_STR(2, orig_opt);
      WRITE_MSG(2, "'\n");
      return false;
    }

    *ptr = val;
  }
  else if (ptr!=0) {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "value already set in '");
    WRITE_STR(2, orig_opt);
    WRITE_MSG(2, "'\n");
    return false;
  }
  else {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "unknown limit in '");
    WRITE_STR(2, orig_opt);
    WRITE_MSG(2, "'\n");
    return false;
  }

  return true;
}

bool
isHigherLimit(uint_least32_t lhs, uint_least32_t rhs)
{
  if (lhs==VC_CDLIM_KEEP || rhs==VC_CDLIM_KEEP) return false;

  return lhs > rhs;
}

int main(int argc, char *argv[])
{
  bool		do_set       = false;
  bool		do_remove    = false;
  xid_t		xid          = VC_NOCTX;
  uint32_t	flags        = 0;
  char		*endptr;
  int		sum          = 0;

  struct vc_ctx_dlimit		limit = {
    .space_used   = VC_CDLIM_KEEP,
    .space_total  = VC_CDLIM_KEEP,
    .inodes_used  = VC_CDLIM_KEEP,
    .inodes_total = VC_CDLIM_KEEP,
    .reserved     = VC_CDLIM_KEEP
  };
  
  while (1) {
    int		c = getopt_long(argc, argv, "+x:s:df:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0]);
      case CMD_VERSION	:  showVersion();
      case 'x'		:  xid = Evc_xidopt2xid(optarg, true); break;
      case 's'		:
	if (!setDLimitField(&limit, optarg))
	  return EXIT_FAILURE;
	else
	  do_set = true;
	break;
      case 'd'		:  do_remove = true; break;
      case 'f'		:
	{
	  flags = strtol(optarg, &endptr, 0);
	  if ((flags == 0 && errno != 0) || *endptr != '\0') {
	    WRITE_MSG(2, "Invalid flags argument: '");
	    WRITE_STR(2, optarg);
	    WRITE_MSG(2, "'; try '--help' for more information\n");
	    return EXIT_FAILURE;
	  }
	}
	break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  sum = ((do_set ? 1 : 0) + (do_remove ? 1 : 0));
  
  if (sum>1)
    WRITE_MSG(2, "Can not specify multiple operations; try '--help' for more information\n");
  else if (optind==argc)
    WRITE_MSG(2, "No mount point specified; try '--help' for more information\n");
  else if (xid==VC_NOCTX)
    WRITE_MSG(2, "No xid specified; try '--help' for more information\n");
  else if (isHigherLimit(limit.space_used, limit.space_total))
    WRITE_MSG(2, "invalid parameters: 'space_used' is larger than 'space_total'\n");
  else if (isHigherLimit(limit.inodes_used, limit.inodes_total))
    WRITE_MSG(2, "invalid parameters: 'inodes_used' is larger than 'inodes_total'\n");
  else {
    for (; optind < argc; ++optind) {
      if      (do_set)     setDlimit(argv[optind], xid, flags, &limit);
      else if (do_remove)  remDlimit(argv[optind], xid, flags);
      else                 printDlimit(argv[optind], xid, flags, true);
    }

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
