// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/*
	Set the global per context limit of a resource (memory, file handle).
	This utility can do it either for the current context or a selected
	one.

	It uses the same options as ulimit, when possible
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include "vserver.h"
#include "internal.h"
#include "util.h"

#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/resource.h>

#define NUMLIM(X) \
{ #X, required_argument, 0, 2048|X }
#define RESLIM(RES,V) \
  { #RES, required_argument, 0, 2048|RLIMIT_##V }

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, 'h' },
  { "version",  no_argument,  0, 'v' },
  { "all",      no_argument,  0, 'a' },
  NUMLIM( 0), NUMLIM( 1), NUMLIM( 2), NUMLIM( 3),
  NUMLIM( 4), NUMLIM( 5), NUMLIM( 6), NUMLIM( 7),
  NUMLIM( 8), NUMLIM( 9), NUMLIM(10), NUMLIM(11),
  NUMLIM(12), NUMLIM(13), NUMLIM(14), NUMLIM(15),
  NUMLIM(16), NUMLIM(17), NUMLIM(18), NUMLIM(19),
  NUMLIM(20), NUMLIM(21), NUMLIM(22), NUMLIM(23),
  NUMLIM(24), NUMLIM(25), NUMLIM(26), NUMLIM(27),
  NUMLIM(28), NUMLIM(29), NUMLIM(30), NUMLIM(31),
  RESLIM(cpu,     CPU),
  RESLIM(fsize,   FSIZE),
  RESLIM(data,    DATA),
  RESLIM(stack,   STACK),
  RESLIM(core,    CORE),
  RESLIM(rss,     RSS),
  RESLIM(nproc,   NPROC),
  RESLIM(nofile,  NOFILE),
  RESLIM(memlock, MEMLOCK),
  RESLIM(as,      AS),
  RESLIM(locks,   LOCKS),
  { 0,0,0,0 }
};

#define REV_RESLIM(X)	[RLIMIT_##X] = #X
static char const * const LIMIT_STR[] = {
  REV_RESLIM(CPU),     REV_RESLIM(FSIZE), REV_RESLIM(DATA),  REV_RESLIM(STACK),
  REV_RESLIM(CORE),    REV_RESLIM(RSS),   REV_RESLIM(NPROC), REV_RESLIM(NOFILE),
  REV_RESLIM(MEMLOCK), REV_RESLIM(AS),    REV_RESLIM(LOCKS)
};

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " -c <xid> [-nd] [-a|--all] [[-MSH] --(<resource>|<nr>) <value>]*\n\n"
	    "Options:\n"
	    "    -c <xid>    ...  operate on context <xid>\n"
	    "    -a|--all    ...  show all available limits\n"
	    "    -n          ...  do not resolve limit-names\n"
	    "    -d          ...  show limits in decimal\n"
	    "    -M          ...  set Minimum limit\n"
	    "    -S          ...  set Soft limit\n"
	    "    -H          ...  set Hard limit (assumed by default, when neither\n"
	    "                     M nor S was requested)\n"
	    "    --<resource>|<nr> <value>\n"
	    "                ...  set specified (MSH) limit for <resource> to <value>\n\n"
	    "Valid values for resource are cpu, fsize, data, stack, core, rss, nproc,\n"
	    "nofile, memlock, as and locks.\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vlimit " VERSION " -- limits context-resources\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static size_t
fmtHex(char *ptr, vc_limit_t lim)
{
  memcpy(ptr, "0x", 2);
  return utilvserver_fmt_xuint64(ptr+2, lim) + 2;
}

static bool		do_resolve = true;
static size_t		(*fmt_func)(char *, vc_limit_t) = fmtHex;

static void *
appendLimit(char *ptr, bool do_it, vc_limit_t lim)
{
  memcpy(ptr, "  ", 2);
  ptr += 2;
  if (do_it) {
    if (lim==VC_LIM_INFINITY) {
      strcpy(ptr, "INF");
      ptr += 3;
    }
    else {
      ptr += (*fmt_func)(ptr, lim);
      *ptr = ' ';
    }
  }
  else {
    memcpy(ptr, "N/A", 3);
    ptr += 3;
  }

  return ptr;
}

static void
showAll(int ctx)
{
  struct vc_rlimit_mask	mask;
  size_t		i;

  if (vc_get_rlimit_mask(ctx, &mask)==-1) {
    perror("vc_get_rlimit_mask()");
    exit(1);
  }

  for (i=0; i<32; ++i) {
    uint32_t		bitmask = (1<<i);
    struct vc_rlimit	limit;
    char		buf[128], *ptr=buf;

    if (((mask.min|mask.soft|mask.hard) & bitmask)==0) continue;
    if (vc_get_rlimit(ctx, i, &limit)==-1) {
      perror("vc_get_rlimit()");
      //continue;
    }

    memset(buf, ' ', sizeof buf);
    if (do_resolve && i<DIM_OF(LIMIT_STR)) {
      size_t		l = strlen(LIMIT_STR[i]);
      memcpy(ptr, LIMIT_STR[i], l);
      ptr += l;
    }
    else {
      ptr += utilvserver_fmt_uint(ptr, i);
      *ptr = ' ';
    }

    ptr  = appendLimit(buf+10, mask.min &bitmask, limit.min);
    ptr  = appendLimit(buf+30, mask.soft&bitmask, limit.soft);
    ptr  = appendLimit(buf+50, mask.hard&bitmask, limit.hard);

    *ptr++ = '\n';
    write(1, buf, ptr-buf);
  }
}

static void
setLimits(int ctx, struct vc_rlimit const limits[], uint32_t mask)
{
  size_t		i;
  for (i=0; i<32; ++i) {
    if ((mask & (1<<i))==0) continue;
    if (vc_set_rlimit(ctx, i, limits+i)) {
      perror("vc_set_rlimit()");
    }
  }
}

int main (int argc, char *argv[])
{
  // overall used limits
  uint32_t		lim_mask = 0;
  int			set_mask = 0;
  struct vc_rlimit	limits[32];
  bool			show_all = false;
  xid_t			ctx      = VC_NOCTX;

  {
    size_t		i;
    for (i=0; i<32; ++i) {
      limits[i].min  = VC_LIM_KEEP;
      limits[i].soft = VC_LIM_KEEP;
      limits[i].hard = VC_LIM_KEEP;
    }
  }
  
  while (1) {
    int		c = getopt_long(argc, argv, "MSHndhvac:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    if (2048<=c && c<2048+32) {
      int		id  = c-2048;
      vc_limit_t	val;
      
      if (strcmp(optarg, "inf")==0) val = VC_LIM_INFINITY;
      else			    val = atoll(optarg);

      if (set_mask==0)  set_mask=4;
      
      if (set_mask & 1) limits[id].min  = val;
      if (set_mask & 2) limits[id].soft = val;
      if (set_mask & 4) limits[id].hard = val;

      lim_mask |= (1<<id);
      set_mask  = 0;
    }
    else switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case 'v'		:  showVersion();
      case 'c'		:  ctx        = atoi(optarg); break;
      case 'a'		:  show_all   = true;         break;
      case 'n'		:  do_resolve = false;        break;
      case 'd'		:  fmt_func   = utilvserver_fmt_uint64; break;
      case 'M'		:
      case 'S'		:
      case 'H'		:
	switch (c) {
	  case 'M'	:  set_mask |= 1; break;
	  case 'S'	:  set_mask |= 2; break;
	  case 'H'	:  set_mask |= 4; break;
	  default	:  assert(false);
	}
	break;
	
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (ctx==VC_NOCTX) {
    WRITE_MSG(2, "No context specified; try '--help' for more information\n");
    return EXIT_FAILURE;
  }

  setLimits(ctx, limits, lim_mask);
  if (show_all) showAll(ctx);

  return EXIT_SUCCESS;
}
