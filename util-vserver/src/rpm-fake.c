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

#include "pathconfig.h"
#include "util.h"

#include <vserver.h>

#include <dlfcn.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <asm/unistd.h>
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
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#ifndef CLONE_NEWNS
#  define CLONE_NEWNS	0x00020000
#endif

#define LIBNAME		"rpm-fake.so"
#define PLATFORM_FILE	"/etc/rpm/platform"

#define INIT(FILE,FUNC)	FUNC##_func = ((__typeof__(FUNC) *) (xdlsym(FILE, #FUNC)))
#define DECLARE(FUNC)	static __typeof__(FUNC) *       FUNC##_func = 0


#define DBG_INIT	0x0001
#define DBG_VARIABLES	0x0002
#define DBG_RESOLVER	0x0004
#define DBG_EXECV	0x0008
#define DBG_VERBOSE0	0x8000
#define DBG_VERBOSE1	(0x4000 | DBG_VERBOSE0)
#define DBG_VERBOSE2	(0x2000 | DBG_VERBOSE1)

static char const *	ctx_s = 0;
static xid_t		ctx   = VC_NOCTX;
static uint32_t		caps  = ~0;
static int		flags = 0;
static char const *	mnts  = 0;
static char const *	root  = 0;
static int		pw_sock   = -1;
static int		sync_sock = -1;
static unsigned int	debug_level = 0;

static bool		is_initialized = false;

DECLARE(execv);
DECLARE(getpwnam);
DECLARE(getgrnam);
DECLARE(endpwent);
DECLARE(endgrent);

static void		initRPMFake() __attribute__((__constructor__));
static void		exitRPMFake() __attribute__((__destructor__));

static inline bool
isDbgLevel(unsigned int level)
{
  return ((debug_level&level)==level);
}

static void *
xdlsym(void *handle, const char *symbol)
{
  void	*res = dlsym(handle, symbol);
  if (res==0) {
    char const	*error = dlerror();
    write(2, symbol, strlen(symbol));
    write(2, ": ", 2);
    write(2, error, strlen(error));
    write(2, "\n", 2);

    _exit(255);
  }

  return res;
}

static void
showHelp()
{
  WRITE_MSG(1,
	    "Usage: LD_PRELOAD=" LIBNAME " <executable> <args>*\n\n"
	    LIBNAME " unterstands the following environment variables:\n"
	    "  $RPM_FAKE_RESOLVER     ...  program which does the NSS resolving (defaults\n"
	    "                              to " RESOLVER_PROG ")\n"
	    "  $RPM_FAKE_RESOLVER_UID ...  uid of the resolver program\n"
	    "  $RPM_FAKE_RESOLVER_GID ...  gid of the resolver program\n"
	    "  $RPM_FAKE_CTX          ...  vserver context which shall be used for resolver\n"
	    "                              and scriptlets\n"
	    "  $RPM_FAKE_CAP          ...  linux capability remove-mask for the context\n"
	    "  $RPM_FAKE_FLAGS        ...  vserver flags of the context\n"
	    "  $RPM_FAKE_CHROOT       ...  directory of the chroot environment\n"
	    "  $RPM_FAKE_NAMESPACE_MOUNTS\n"
	    "                          ... colon separated list of directories which will\n"
            "                              umounted before scriptlet execution\n\n"
	    "  $RPM_FAKE_HELP          ... shows this message\n"
	    "  $RPM_FAKE_VERSION       ... shows the version of this program\n\n"
	    "  $RPM_FAKE_DEBUG         ... sets the debuglevel bitmask\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    LIBNAME " " VERSION " -- wrapper around rpm\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

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

#if 0
static void
initPwSocket()
{
  char const *	sock_name = getenv("RPM_FAKE_PWSOCKET");
  if (sock_name!=0) {
    int flag;
    struct sockaddr_un	addr = {
      .sun_family = AF_UNIX,
    };

    strncpy(addr.sun_path, sock_name, sizeof(addr.sun_path)-1);
    addr.sun_path[sizeof(addr.sun_path)-1]='\0';
    
    if ((pw_sock=socket(AF_UNIX, SOCK_STREAM, 0))==-1 ||
	connect(pw_sock, (struct sockaddr *)(&addr), sizeof addr)==-1 ||
	(flag=fcntl(pw_sock, F_GETFD))==-1 ||
	fcntl(pw_sock, F_SETFD, flag | FD_CLOEXEC)==-1) {
      perror("error while initializing pw-socket");
      exit(255);
    }
  }
  unsetenv("RPM_FAKE_PWSOCKET");
}
#else
static void
initPwSocket()
{
  char const *	resolver = getenv("RPM_FAKE_RESOLVER");
  if (resolver==0) resolver=RESOLVER_PROG;
  
  if (resolver!=0 && *resolver!='\0') {
    int			res_sock[2];
    int			sync_pipe[2];
    pid_t		pid;
    char const *	uid=0;
    char const *	gid=0;

    uid=getenv("RPM_FAKE_RESOLVER_UID");
    gid=getenv("RPM_FAKE_RESOLVER_GID");

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, res_sock)==-1 ||
	pipe(sync_pipe)==-1 ||
	fcntl(res_sock[0],  F_SETFD, FD_CLOEXEC)==-1 ||
	fcntl(sync_pipe[0], F_SETFD, FD_CLOEXEC)==-1) {
      perror("rpm-fake.so: failed to create/initialize resolver-socket or pipe");
      exit(255);
    }

    pid = fork();
    if (pid==-1) {
      perror("rpm-fake.so: fork()");
      exit(255);
    }

    if (pid==0) {
      char const	*args[10];
      char const	**ptr  = args;
      char const 	*env[] = { "HOME=/", "PATH=/bin:/usr/bin", 0 };
      
      setsid();
      dup2(res_sock[1],  0);
      dup2(res_sock[1],  1);
      if (sync_pipe[1]!=3) {
	close(3);
	dup2(sync_pipe[1], 3);
	close(sync_pipe[1]);
      }
      close(res_sock[1]);
	/* ... *socket[0] are marked as close-on-exec ...*/

      *ptr++ = resolver;
      if (root)  { *ptr++ = "-r"; *ptr++ = root;  }
      if (uid)   { *ptr++ = "-u"; *ptr++ = uid;   }
      if (gid)   { *ptr++ = "-g"; *ptr++ = gid;   }
      if (ctx_s) { *ptr++ = "-c"; *ptr++ = ctx_s; }
      *ptr++ = 0;
      
      execve(resolver, (char **)args, (char **)env);
      perror("rpm-fake.so: failed to exec resolver");
      exit(255);
    }
    else {
      uint8_t		c;

      close(res_sock[1]);
      close(sync_pipe[1]);
      pw_sock   = res_sock[0];
      sync_sock = sync_pipe[0];

      if (read(sync_sock, &c, 1)!=1 ||
	  write(pw_sock, ".", 1)!=1 ||
	  read(pw_sock, &c,   1)!=1 ||
	  c!='.') {
	WRITE_MSG(2, "rpm-fake.so: failed to initialize communication with resolver\n");
	exit(255);
      }

      if (wait4(pid, 0, WNOHANG,0)==-1) {
	WRITE_MSG(2, "rpm-fake.so: unexpected initialization-error of resolver\n");
	exit(255);
      }
    }

    free((char *)ctx_s);
    unsetenv("RPM_FAKE_RESOLVER_GID");
    unsetenv("RPM_FAKE_RESOLVER_UID");
    unsetenv("RPM_FAKE_RESOLVER");    
  }
}
#endif

static void
initEnvironment()
{
  int		syscall_rev;
  int		syscall_nr;
  
  if (is_initialized) return;

  syscall_rev = getAndClearEnv("RPM_FAKE_S_CONTEXT_REV", 0);
  syscall_nr  = getAndClearEnv("RPM_FAKE_S_CONTEXT_NR",  273);
  
#ifdef VC_ENABLE_API_LEGACY
  {
    extern void vc_init_internal_legacy(int ctx_rev, int ctx_number,
					int ipv4_rev, int ipv4_number);
  
    vc_init_internal_legacy(syscall_rev, syscall_nr, 3, 274);
  }
#endif

  ctx_s = getenv("RPM_FAKE_CTX");
  if (ctx_s && *ctx_s) ctx_s = strdup(ctx_s);
  else                 ctx_s = 0;

  ctx       = getAndClearEnv("RPM_FAKE_CTX",  VC_RANDCTX);
  caps      = getAndClearEnv("RPM_FAKE_CAP",  ~0x3404040f);
  flags     = getAndClearEnv("RPM_FAKE_FLAGS", 0);
  root      = getenv("RPM_FAKE_CHROOT");
  mnts      = getenv("RPM_FAKE_NAMESPACE_MOUNTS");
  if (mnts && *mnts) mnts = strdup(mnts);
  else               mnts = 0;

  unsetenv("RPM_FAKE_CHROOT");
  unsetenv("RPM_FAKE_NAMESPACE_MOUNTS");

  
  is_initialized = true;
}

static void
initSymbols()
{
  INIT(RTLD_NEXT, execv);
  INIT(RTLD_NEXT, getgrnam);
  INIT(RTLD_NEXT, getpwnam);
  INIT(RTLD_NEXT, endpwent);
  INIT(RTLD_NEXT, endgrent);
}

static void
fixPreloadEnv()
{
  char			*env = getenv("LD_PRELOAD");
  char			*pos;

    // the const <-> non-const assignment is not an issue since the following
    // modifying operations will not be executed in the const-case
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
  if (isDbgLevel(DBG_VERBOSE1|DBG_VARIABLES)) {
    WRITE_MSG(2, "env='");
    WRITE_STR(2, env);
    WRITE_MSG(2, "'\n");
  }
#endif

  if (*env=='\0') unsetenv("LD_PRELOAD");
}

void
initRPMFake()
{
  if (getenv("RPM_FAKE_VERSION")) showVersion();
  if (getenv("RPM_FAKE_HELP"))    showHelp();
  
  debug_level = getAndClearEnv("RPM_FAKE_DEBUG", 0);

  if (isDbgLevel(DBG_INIT)) WRITE_MSG(2, ">>>>> initRPMFake <<<<<\n");
  
  fixPreloadEnv();
  initSymbols();
  initEnvironment();
  initPwSocket();

#if 0
  if (isDbgLevel(DBG_VARIABLES|DBG_VERBOSE2)) {
    
  }
#endif
}

void
exitRPMFake()
{ 
  if (isDbgLevel(DBG_INIT)) WRITE_MSG(2, ">>>>> exitRPMFake <<<<<\n");
  if (pw_sock!=-1) {
    uint8_t	c;
    read(sync_sock, &c, 1);
    write(pw_sock, "Q", 1);
  }
}


  //============   the worker part   ===========


static bool
doPwStringRequest(uint32_t *result, char style, char const *name)
{
  uint32_t	len = strlen(name);
  uint8_t	code;
  uint8_t	c;

    // read the token...
  read(sync_sock, &c, 1);

  write(pw_sock, &style, 1);
  write(pw_sock, &len,   sizeof len);
  write(pw_sock, name,   len);
  read (pw_sock, &code,  sizeof code);
  read (pw_sock, result, sizeof *result);

  return code!=0;
}

struct passwd *
getpwnam(const char * name)
{
  if (pw_sock==-1) return getpwnam_func(name);
  else {
    static struct passwd	res = {
      .pw_passwd = "*",
      .pw_gid    = -1,
      .pw_gecos  = "",
      .pw_dir    = "/",
      .pw_shell  = "/bin/false"
    };

    res.pw_name = (char *)(name);
    if (!doPwStringRequest(&res.pw_uid, 'P', name)) return 0;
    
    return &res;
  }
}

struct group *
getgrnam(const char * name)
{
  if (pw_sock==-1) return getgrnam_func(name);
  else {
    static struct group		res = {
      .gr_passwd = "*",
      .gr_mem    = 0
    };

    res.gr_name = (char *)(name);
    if (!doPwStringRequest(&res.gr_gid, 'G', name)) return 0;

    return &res;
  }
}

void
endgrent()
{
  if (pw_sock==-1) endgrent_func();
}

void
endpwent()
{
  if (pw_sock==-1) endpwent_func();
}


static int
execvWorker(char const *path, char * const argv[])
{
  int		res;

  if ((res=vc_new_s_context(ctx,caps,flags))!=-1 &&
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
removeNamespaceMountsChild(struct ExecvParams const *params)
{
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

  return execvWorker(params->path, params->argv);
}

static int
removeNamespaceMounts(char const *path, char * const argv[])
{
  if (mnts==0) return execvWorker(path, argv);

  {
    int				status;
    pid_t			p, pid;
    struct ExecvParams		params;

    params.path = path;
    params.argv = argv;
    params.mnts = mnts;

      // the rpmlib signal-handler is still active; use the default one to
      // make wait4() working...
    signal(SIGCHLD, SIG_DFL);

#ifdef NDEBUG    
    pid = syscall(__NR_clone, CLONE_NEWNS|SIGCHLD|CLONE_VFORK, 0);
#else
    pid = syscall(__NR_clone, CLONE_NEWNS|SIGCHLD, 0);
#endif

    switch (pid) {
      case -1	:  return -1;
      case 0	:  _exit(removeNamespaceMountsChild(&params));
      default	:  break;
    }
	
    while ((p=wait4(pid, &status, 0,0))==-1 &&
	   (errno==EINTR || errno==EAGAIN)) ;

    if (p==-1)   return -1;

    if (WIFEXITED(status))   _exit(WEXITSTATUS(status));
    if (WIFSIGNALED(status)) kill(getpid(), WTERMSIG(status));

    return -1;
  }
}


int
execv(char const *path, char * const argv[]) __THROW
{
  return removeNamespaceMounts(path, argv);
}
