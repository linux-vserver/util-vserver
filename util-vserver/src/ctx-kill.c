// $Id$    --*- c++ -*--

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
#include "compat.h"

#include "wrappers.h"
#include "wrappers-vserver.h"
#include "util.h"
#include "stack-start.h"

#include <time.h>
#include <dirent.h>
#include <getopt.h>
#include <signal.h>
#include <sched.h>
#include <assert.h>

#ifndef CLONE_NEWNS
#  define CLONE_NEWNS 0x00020000
#endif

#ifndef MNT_DETACH
#  define MNT_DETACH	0x000000002
#endif

#if 0
#  include <stdio.h>
#  define DPRINTF(...)	printf(__VA_ARGS__)
#else
#  define DPRINTF(...)	(void)0
#endif

struct ArgInfo {
    enum { tpUNSET, tpCTX, tpPID }	type;
    xid_t		ctx;
    pid_t		pid;
    unsigned int	interval;
    bool		shutdown;
    bool		omit_init;
    size_t		argc;
    char * const *	argv;
};

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, 'h' },
  { "version",  no_argument,  0, 'v' },
  { "all",      no_argument,       0, 'a' },
  { "ctx",      required_argument, 0, 'c' },
  { "pid",      required_argument, 0, 'p' },
  { "interval", required_argument, 0, 'i' },
  { "shutdown", no_argument,       0, 's' },
  { 0,0,0,0 }
};

int	wrapper_exit_code = 1;

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " (-c|--ctx <ctx>)|(-p|--pid <pid>) [-i|--interval <msec>] [-a|--all]\n"
	    "                [-s|--shutdown] [--] <signal> [<sec> <signal>]*\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "ctx-kill " VERSION " -- kills processes in foreign contexts\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
iterateThroughProc(void (*fn)(pid_t pid, void *data),
		   struct ArgInfo const * const args,
		   void *data)
{
  int		grp  = getpgrp();
  DIR		*dir;
  struct dirent	*d;

  kill(-1, SIGSTOP);
  dir = opendir("/proc");
  if (dir==0) {
    perror("opendir()");
    kill(-1, SIGCONT);
    exit(1);
  }
    
  kill(-1, SIGSTOP);
  while ((d=readdir(dir))!=0) {
    pid_t	pid = atoi(d->d_name);
    if (pid==0 ||			// not a /proc/<pid>/ entry
	(args->omit_init && pid==1) ||
	getpgid(pid)==grp)		// our own process-group
      continue;

    (*fn)(pid,data);
  }
  kill(-1, SIGCONT);

  closedir(dir);
}

static void
doKillAllSingle(pid_t pid, void *sig_ptr_v)
{
  register int const * const	sig_ptr = sig_ptr_v;
  
  DPRINTF("sending signal '%u' to process '%u'\n",
	  *sig_ptr, pid);
  kill(pid, *sig_ptr);
}

static void
getRemainingProcCountProc(pid_t UNUSED pid, void *cnt_v)
{
  register size_t * const	cnt = cnt_v;
  
  ++*cnt;
}

static size_t
getRemainingProcCount(struct ArgInfo const * const args)
{
  size_t	cnt = 0;
  iterateThroughProc(&getRemainingProcCountProc, args, &cnt);

  return cnt;
}

static void
doKillAll(struct ArgInfo const * const args)
{
  size_t	i=0;
  pid_t		init_pid = vc_X_getinitpid(0);

  signal(SIGSTOP, SIG_IGN);

  while (i<args->argc) {
    int		sig    	  = atoi(args->argv[i]);
    time_t	next_time = time(0);

    ++i;
    if (i<args->argc)
      next_time += atoi(args->argv[i]);
    
    iterateThroughProc(&doKillAllSingle, args, &sig);
    if (init_pid!=-1 && init_pid>1 && init_pid!=getpid())
      kill(init_pid, sig);

    while (time(0)<next_time && getRemainingProcCount(args)>0)
      usleep(args->interval * 1000);

    ++i;
  }

  if (args->shutdown && getRemainingProcCount(args)>0) {
    WRITE_MSG(2, "There are processes which could not be killed\n");
    exit(1);
  }

}

static void
doKillSingle(struct ArgInfo const * const args)
{
  size_t	i=0;

  while (i<args->argc) {
    int		sig    	  = atoi(args->argv[i]);
    time_t	next_time = time(0);

    ++i;
    if (i<args->argc)
      next_time += atoi(args->argv[i]);

    DPRINTF("sending signal '%u' to process '%u'\n",
	    sig, args->pid);
    kill(args->pid, sig);
    while (time(0)<next_time &&
	   (getpgid(args->pid)!=-1 || errno!=ESRCH))
      
      usleep(args->interval * 1000);

    ++i;
  }
}

static int
childFunc(void *args_v)
{
  struct ArgInfo *	args = args_v;

  Emount("none", "/mnt", "tmpfs", MS_NODEV|MS_NOEXEC|MS_NOSUID, "size=4k");
  Echdir("/mnt");
  Emkdir("proc", 0600);
  Emkdir("old",  0600);
  Epivot_root(".", "old");
  Echroot(".");
  Eumount2("old", MNT_DETACH);
  Emkdir("old/foo",0600);
  Emount("none", "/proc", "proc", 0, 0);

    // TODO: drop additional capabilities?
  
    // TODO: it may be be better for ctx-shutdown to:
    // * generate ctx with S_CTX_INFO_INIT
    // * send kill to -1
  Evc_new_s_context(args->ctx, ~0, S_CTX_INFO_LOCK|S_CTX_INFO_SCHED);

  switch (args->type) {
    case tpCTX	:  doKillAll(args); break;
    case tpPID	:  doKillSingle(args); break;
    default	:  assert(false); return 1;
  }

  return 0;
}
  
static xid_t
determineContext(pid_t pid)
{
  int		fd[2];
  pid_t		chld;
  xid_t		res;
  
  Epipe(fd);
  chld = Efork();

  if (chld==0) {
    Eclose(fd[0]);
    Evc_new_s_context(1, ~0, S_CTX_INFO_LOCK|S_CTX_INFO_SCHED);
    res = vc_X_getctx(pid);
    Ewrite(fd[1], &res, sizeof res);
    Eclose(fd[1]);
    _exit(0);
  }
  
  Eclose(fd[1]);
  if (Eread(fd[0], &res, sizeof res)!=sizeof(res)) {
    WRITE_MSG(2, "internal communication error while determining ctx\n");
    exit(1);
  }
  Eclose(fd[0]);
  if (res==VC_NOCTX) {
    WRITE_MSG(2, "can not determine contex for given pid\n");
    exit(1);
  }

  return res;
}

int main(int argc, char *argv[])
{
  pid_t			pid, p;
  char			buf[0x8000];
  int			status;
  struct ArgInfo	args = {
    .type      = tpUNSET,
    .interval  = 500,
    .shutdown  = false,
    .omit_init = true };

  while (1) {
    int		c = getopt_long(argc, argv, "hvsac:p:i:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case 'v'		:  showVersion();
      case 'c'		:
	args.type = tpCTX;
	args.ctx  = atoi(optarg);
	break;
      case 'p'		:
	args.type = tpPID;
	args.pid  = atoi(optarg);
	break;
      case 'i'		:  args.interval  = atoi(optarg); break;
      case 's'		:  args.shutdown  = true; break;
      case 'a'		:  args.omit_init = false; break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (args.type==tpUNSET){
    WRITE_MSG(2, "Please specify a context or a pid\n");
    return EXIT_FAILURE;
  }

  if (optind==argc) {
    WRITE_MSG(2, "No signal specified\n");
    return EXIT_FAILURE;
  }

  if (args.type==tpPID)
    args.ctx = determineContext(args.pid);

  if (args.ctx==0) {
    WRITE_MSG(2, "Can not operate in ctx 0\n");
    return EXIT_FAILURE;
  }
  
  args.argc = argc-optind;
  args.argv = argv+optind;

    // TODO: args.argv verification (only integers)

  if (args.argc%2 != 1) {
    WRITE_MSG(2, "Wrong count of '<signal> [<pause> <signal>]*' arguments specified.\n");
    return EXIT_FAILURE;
  }

  pid = Eclone(childFunc, STACK_START(buf), CLONE_NEWNS|SIGCHLD, &args);
  p   = Ewait4(pid, &status, 0,0);

  if (WIFEXITED(status))   return WEXITSTATUS(status);
  if (WIFSIGNALED(status)) return kill(getpid(), WTERMSIG(status));
  return EXIT_FAILURE;
}
