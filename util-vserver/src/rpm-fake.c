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
//  

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include <vserver.h>

#include <dlfcn.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include <sched.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LIBNAME		"rpm-fake.so"
#define PLATFORM_FILE	"/etc/rpm/platform"

#define INIT(FILE,FUNC)	FUNC##_func = ((__typeof__(FUNC) *) (dlsym(FILE, #FUNC)))
#define DECLARE(FUNC)	static __typeof__(FUNC) *       FUNC##_func = 0

static bool		is_initialized = false;
DECLARE(execv);
  //DECLARE(open);

static int
getAndClearEnv(char const *key, int dflt)
{
  char		*env = getenv(key);
  int		res;

  if (env==0 || env[0]=='\0') res = dflt;
  else {
    res = atoi(env);
    unsetenv(key);
  }

  return res;
}

static void
initLib()
{
  if (is_initialized) return;

  INIT(RTLD_NEXT, execv);
    //INIT(RTLD_NEXT, open);

  is_initialized = true;
}

static void
fixPreloadEnv()
{
  char			*env = getenv("LD_PRELOAD");
  char			*pos;

    // the const <-> non-const assignment is not an issue since the following modifying operations
    // will not be executed in the const-case
  env = env ? env : "";
  pos = strstr(env, LIBNAME);

  if (pos!=0) {
    char	*end_pos = pos + sizeof(LIBNAME);
    bool	is_end = (end_pos[-1]=='\0');
    char	*start_pos;

    end_pos[-1] = '\0';
    start_pos   = strrchr(env, ':');
    if (start_pos==0) start_pos = env;
    else if (!is_end) ++start_pos;

    if (is_end) *start_pos = '\0';
    else        memmove(start_pos, end_pos, strlen(end_pos)+1);
  }

#ifdef DEBUG
  printf("env='%s'\n", env);
#endif

  if (*env=='\0') unsetenv("LD_PRELOAD");
}

static int
execvWorker(char const *path, char * const argv[])
{
  int		res;
  int		ctx;

  ctx = getAndClearEnv("RPM_FAKE_CTX",  -1);
  if ( (res=vc_new_s_context(ctx,
			     getAndClearEnv("RPM_FAKE_CAP",  ~0x3404040f),
			     getAndClearEnv("RPM_FAKE_FLAGS", 0)))!=-1 &&
       (res=execv_func(path, argv)!=-1) ) {}

  return res;
}

struct ExecvParams
{
    char const *	path;
    char * const *	argv;
    char const *	mnts;
};

static int
removeNamespaceMountsChild(void *params_v)
{
  struct ExecvParams *	params = params_v;
  char			buf[strlen(params->mnts)+1], *ptr;

  strcpy(buf, params->mnts);
  ptr = strtok(buf, ":");
  while (ptr) {
    if (umount2(ptr, 0)==-1) {
	// FIXME: What is the semantic for CLONE_NEWNS? Is it ok that mounts in
	// chroots are visible only, when chroot is on /dev/root?
	//
	// For now, ignore any errors, but future versions should handle them.

	//return -1;
    }
    ptr = strtok(0, ":");
  }

  unsetenv("RPM_FAKE_NAMESPACE_MOUNTS");
  return execvWorker(params->path, params->argv);
}

static int
removeNamespaceMounts(char const *path, char * const argv[])
{
  char const *	mnts = getenv("RPM_FAKE_NAMESPACE_MOUNTS");

  if (mnts==0) return execvWorker(path, argv);

  {
    char	buf[512 + 2*strlen(mnts)];
    int		status;
    pid_t	p;
    struct ExecvParams		params = {
      .path = path,
      .argv = argv,
      .mnts = mnts,
    };

      // the rpmlib signal-handler is still active; use the default one to
      // make wait4() working...
    signal(SIGCHLD, SIG_DFL);

    pid_t	pid = clone(removeNamespaceMountsChild, buf+sizeof(buf)/2,
			    CLONE_NEWNS|SIGCHLD|CLONE_VFORK, &params);

    if (pid==-1) return -1;
    while ((p=wait4(pid, &status, 0,0))==-1 &&
	   (errno==EINTR || errno==EAGAIN)) ;

    if (p==-1)   return -1;

    if (WIFEXITED(status))   exit(WEXITSTATUS(status));
    if (WIFSIGNALED(status)) kill(getpid(), WTERMSIG(status));

    return -1;
  }
}

int
execv(char const *path, char * const argv[]) __THROW
{
  initLib();
  fixPreloadEnv();
  return removeNamespaceMounts(path, argv);
}
