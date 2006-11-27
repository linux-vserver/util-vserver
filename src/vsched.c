// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// Copyright (C) 2006 Daniel Hokka Zakrisson <daniel@hozac.com>
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
#include "vserver.h"

#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stddef.h>

#define ENSC_WRAPPERS_PREFIX	"vsched: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_DIRENT	1
#define ENSC_WRAPPERS_STAT	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_XID			0x4000
#define CMD_FRATE		0x4001
#define CMD_INTERVAL		0x4002
#define CMD_TOKENS		0x4003
#define CMD_TOK_MIN		0x4004
#define CMD_TOK_MAX		0x4005
#define CMD_CPU_MASK		0x4006
#define CMD_PRIO_BIAS		0x4007
#define CMD_FRATE2		0x4008
#define CMD_INTERVAL2		0x4009
#define CMD_CPUID		0x400a
#define CMD_BUCKETID		0x400b
#define CMD_FORCE		0x400c
#define CMD_IDLE_TIME		0x400d
#define CMD_DIR			0x400e
#define CMD_MISSING		0x400f

int			wrapper_exit_code = 255;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "ctx",           required_argument, 0, CMD_XID },
  { "xid",           required_argument, 0, CMD_XID },
  { "fill-rate",     required_argument, 0, CMD_FRATE },
  { "interval",      required_argument, 0, CMD_INTERVAL },
  { "tokens",        required_argument, 0, CMD_TOKENS },
  { "tokens_min",    required_argument, 0, CMD_TOK_MIN },
  { "tokens-min",    required_argument, 0, CMD_TOK_MIN },
  { "tokens_max",    required_argument, 0, CMD_TOK_MAX },
  { "tokens-max",    required_argument, 0, CMD_TOK_MAX },
  { "prio_bias",     required_argument, 0, CMD_PRIO_BIAS },
  { "prio-bias",     required_argument, 0, CMD_PRIO_BIAS },
  { "priority_bias", required_argument, 0, CMD_PRIO_BIAS },
  { "priority-bias", required_argument, 0, CMD_PRIO_BIAS },
  { "cpu_mask",      required_argument, 0, CMD_CPU_MASK },
  { "fill-rate2",    required_argument, 0, CMD_FRATE2 },
  { "interval2",     required_argument, 0, CMD_INTERVAL2 },
  { "cpu-id",        required_argument, 0, CMD_CPUID },
  { "bucket-id",     required_argument, 0, CMD_BUCKETID },
  { "force",         no_argument,       0, CMD_FORCE },
  { "idle-time",     no_argument,       0, CMD_IDLE_TIME },
  { "dir",           required_argument, 0, CMD_DIR },
  { "missingok",     no_argument,       0, CMD_MISSING },
  {0,0,0,0}
};

struct sched_opt {
  const char * const	name;
  uint_least32_t	mask;
  size_t		offset;
};
#define FOPT(NAME,MASK,FIELD)	{ #NAME, MASK, offsetof(struct vc_set_sched, FIELD) }
static struct sched_opt FILE_OPTIONS[] = {
  FOPT(fill-rate,	VC_VXSM_FILL_RATE,			fill_rate),
  FOPT(interval,	VC_VXSM_INTERVAL,			interval),
  FOPT(tokens,		VC_VXSM_TOKENS,				tokens),
  FOPT(tokens-min,	VC_VXSM_TOKENS_MIN,			tokens_min),
  FOPT(tokens-max,	VC_VXSM_TOKENS_MAX,			tokens_max),
  FOPT(prio-bias,	VC_VXSM_PRIO_BIAS,			priority_bias),
  FOPT(priority-bias,	VC_VXSM_PRIO_BIAS,			priority_bias),
  FOPT(fill-rate2,	VC_VXSM_FILL_RATE2|VC_VXSM_IDLE_TIME,	fill_rate2),
  FOPT(interval2,	VC_VXSM_INTERVAL2|VC_VXSM_IDLE_TIME,	interval2),
  FOPT(cpu-id,		VC_VXSM_CPU_ID,				cpu_id),
  FOPT(bucket-id,	VC_VXSM_BUCKET_ID,			bucket_id),
  FOPT(idle-time,	VC_VXSM_IDLE_TIME,			set_mask),
  {0,0,0}
};

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);

  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--xid <xid>] <sched options>* [--dir <dir>] [--] [<command> <args>*]\n"
	    "\n"
	    "Options:\n"
	    "    --fill-rate <rate>\n"
	    "    --interval <interval>\n"
	    "    --tokens <tokens>\n"
	    "    --tokens-min <tokens>\n"
	    "    --tokens-max <tokens>\n"
	    "    --prio-bias <bias>\n"
	    "    --fill-rate2 <rate>\n"
	    "    --interval2 <interval>\n"
	    "    --cpu-id <CPU id>\n"
	    "    --bucket-id <bucket id>\n"
	    "    --idle-time    ...  set the idle time flag; this is required for\n"
	    "                        all updates to the scheduler to keep it enabled\n"
	    "    --force        ...  force update of all per-CPU schedulers now\n"
	    "    --dir <dir>    ...  read settings from <dir>\n"
	    "    --missingok    ...  do not fail when <dir> does not exist\n"
	    "\nPlease report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vsched " VERSION " -- modifies scheduling parameters\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004 Enrico Scholz\n"
	    "Copyright (C) 2006 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void do_dir_entry(struct vc_set_sched *sched, const char *name)
{
  int fd;
  char buf[128];
  signed long val;
  struct sched_opt *opt;
  ssize_t len;
  char *newline;

  for (opt = FILE_OPTIONS; opt->name != 0; opt++) {
    if (strcmp(name, opt->name) == 0)
      break;
  }
  if (opt->name == 0)
    return;

  fd = Eopen(name, O_RDONLY, 0);
  len = Eread(fd, buf, sizeof(buf)-1);
  Eclose(fd);
  buf[len] = '\0';
  if ((newline=strchr(buf, '\n')) != NULL)
    *newline = '\0';

  if (!isNumber(buf, &val, true)) {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX);
    WRITE_STR(2, name);
    WRITE_MSG(2, ": is not a number\n");
    exit(1);
  }

  if (opt->offset != offsetof(struct vc_set_sched, set_mask))
    *(int_least32_t *)(((char *)sched)+opt->offset) = (int_least32_t) val;

  sched->set_mask |= opt->mask;
}

static void do_dir(xid_t xid, struct vc_set_sched *sched, const char *dir, int missing_ok, int per_cpu)
{
  DIR			*dp;
  struct dirent		*de;
  int			cur_fd = Eopen(".", O_RDONLY, 0);
  struct stat		st;

  if (chdir(dir)!=-1) {
    dp = Eopendir(".");
    while ((de = Ereaddir(dp)) != NULL) {
      if (de->d_name[0] == '.' && (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0')))
	continue;
      Estat(de->d_name, &st);
      if (S_ISDIR(st.st_mode))
	continue;
      do_dir_entry(sched, de->d_name);
    }

    /* set the values now */
    if (vc_set_sched(xid, sched) == -1) {
      perror(ENSC_WRAPPERS_PREFIX "vc_set_sched()");
      exit(1);
    }

    if (!per_cpu) {
      struct vc_set_sched per_cpu_sched;

      rewinddir(dp);
      while ((de = Ereaddir(dp)) != NULL) {
	if (de->d_name[0] == '.' && (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0')))
	  continue;
	Estat(de->d_name, &st);
	if (S_ISDIR(st.st_mode)) {
	  per_cpu_sched.set_mask = sched->set_mask & (VC_VXSM_IDLE_TIME|VC_VXSM_FORCE);
	  do_dir(xid, &per_cpu_sched, de->d_name, 0, 1);
	}
      }
    }

    Eclosedir(dp);
  }
  else if (!missing_ok) {
    perror(ENSC_WRAPPERS_PREFIX "chdir()");
    exit(wrapper_exit_code);
  }

  Efchdir(cur_fd);
}

#define SETVAL(ATTR,MASK) \
  if (!isNumber(optarg, &tmp, false)) { \
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "non-numeric value specified for '--" #ATTR "'\n"); \
    exit(wrapper_exit_code); \
  } \
  else { \
    sched.ATTR      = tmp; \
    sched.set_mask |= MASK; \
  }

int main(int argc, char *argv[])
{
  xid_t			xid   = VC_NOCTX;
  signed long		tmp;
  struct vc_set_sched	sched = {
    .set_mask = 0
  };
  const char		*dir = NULL;
  int			missing_ok = 0;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_XID	:  xid = Evc_xidopt2xid(optarg,true);         break;
      case CMD_FRATE	:  SETVAL(fill_rate,     VC_VXSM_FILL_RATE);  break;
      case CMD_INTERVAL	:  SETVAL(interval,      VC_VXSM_INTERVAL);   break;
      case CMD_TOKENS	:  SETVAL(tokens,        VC_VXSM_TOKENS);     break;
      case CMD_TOK_MIN	:  SETVAL(tokens_min,    VC_VXSM_TOKENS_MIN); break;
      case CMD_TOK_MAX	:  SETVAL(tokens_max,    VC_VXSM_TOKENS_MAX); break;
      case CMD_PRIO_BIAS:  SETVAL(priority_bias, VC_VXSM_PRIO_BIAS);  break;
      case CMD_CPU_MASK	:
	WRITE_MSG(2, "vsched: WARNING: the '--cpu_mask' parameter is deprecated and will not have any effect\n");
	break;
      case CMD_FRATE2	:  SETVAL(fill_rate2,    VC_VXSM_FILL_RATE2); break;
      case CMD_INTERVAL2:  SETVAL(interval2,     VC_VXSM_INTERVAL2);  break;
      case CMD_CPUID	:  SETVAL(cpu_id,        VC_VXSM_CPU_ID);     break;
      case CMD_BUCKETID	:  SETVAL(bucket_id,     VC_VXSM_BUCKET_ID);  break;
      case CMD_DIR	:  dir = optarg;                              break;
      case CMD_MISSING	:  missing_ok = 1;                            break;
      case CMD_FORCE	:  sched.set_mask |= VC_VXSM_FORCE;           break;
      case CMD_IDLE_TIME:  sched.set_mask |= VC_VXSM_IDLE_TIME;       break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (xid==VC_NOCTX && optind==argc) {
    WRITE_MSG(2, "Without a program, '--xid' must be used; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  if (sched.set_mask==0 && dir==NULL && optind==argc) {
    WRITE_MSG(2, "Neither an option nor a program was specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  if (xid==VC_NOCTX)
    xid = Evc_get_task_xid(0);

  if (dir) {
    do_dir(xid, &sched, dir, missing_ok, 0);
  }
  else {
    if (sched.set_mask!=0 && vc_set_sched(xid, &sched)==-1) {
      perror("vc_set_sched()");
      exit(255);
    }
  }

  if (optind<argc)
    EexecvpD(argv[optind],argv+optind);

  return EXIT_SUCCESS;
}
