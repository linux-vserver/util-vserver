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

#include <lib/vserver.h>
#include <lib/internal.h>
#include <lib_internal/sys_unshare.h>

#include <sys/socket.h>
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
#include <sys/un.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <sched.h>

  // from selinux.h
  // FIXME: add configure autodetection and include <selinux.h> directly
int rpm_execcon(unsigned int verified,
		const char *filename,
		char *const argv[], char *const envp[]);


#define ENSC_WRAPPERS_PREFIX	"rpm-fake.so: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

#undef _POSIX_SOURCE
#include "capability-compat.h"

#define LIBNAME		"rpm-fake.so"
#define PLATFORM_FILE	"/etc/rpm/platform"

#define INIT(FILE,FUNC)	FUNC##_func = ((__typeof__(FUNC) *) (xdlsym(FILE, #FUNC)))
#define DECLARE(FUNC)	static __typeof__(FUNC) *	FUNC##_func = 0

#define DEBUG 1

#define DBG_INIT	0x0001
#define DBG_VARIABLES	0x0002
#define DBG_RESOLVER	0x0004
#define DBG_EXECV	0x0008
#define DBG_ENV		0x0010
#define DBG_VERBOSE0	0x8000
#define DBG_VERBOSE1	(0x4000 | DBG_VERBOSE0)
#define DBG_VERBOSE2	(0x2000 | DBG_VERBOSE1)

int			wrapper_exit_code = 255;

static xid_t		ctx   = VC_NOCTX;
static uint32_t		caps  = ~0;
static int		flags = 0;
static char const *	mnts  = 0;
static char const *	root  = 0;
static int		pw_sock   = -1;
static int		sync_sock = -1;
static unsigned int	debug_level = 0;

static bool		is_initialized = false;

static bool		ctx_created = false;

  //DECLARE(rpm_execcon);
  //DECLARE(execv);
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
    Vwrite(2, symbol, strlen(symbol));
    Vwrite(2, ": ", 2);
    Vwrite(2, error, strlen(error));
    Vwrite(2, "\n", 2);

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

static void
unsetPreloadEnv()
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

static void
clearEnv()
{
  if (isDbgLevel(DBG_ENV)) WRITE_MSG(2, "clearEnv()\n");
  
  unsetenv("RPM_FAKE_S_CONTEXT_REV");
  unsetenv("RPM_FAKE_S_CONTEXT_NR");
  unsetenv("RPM_FAKE_CTX");
  unsetenv("RPM_FAKE_FLAGS");
  unsetenv("RPM_FAKE_CHROOT");
  unsetenv("RPM_FAKE_NAMESPACE_MOUNTS");

  unsetenv("RPM_FAKE_RESOLVER_GID");
  unsetenv("RPM_FAKE_RESOLVER_UID");
  unsetenv("RPM_FAKE_RESOLVER");    
  unsetenv("RPM_FAKE_PWSOCKET");

  unsetenv("RPM_FAKE_DEBUG");

  unsetPreloadEnv();
}

static int
getDefaultEnv(char const *key, int dflt)
{
  char		*env = getenv(key);
  int		res;

  if (env==0 || env[0]=='\0') res = dflt;
  else                        res = atoi(env);

  return res;
}

  /// \returns true iff we are in ctx after leaving this function
static bool
setupContext(xid_t xid, char const **xid_str)
{
  bool		res = false;
  
  if (vc_isSupported(vcFEATURE_MIGRATE)) {
    xid_t	rc=VC_NOCTX;

    if ((xid==VC_DYNAMIC_XID || !vc_is_dynamic_xid(xid)) &&
	(rc=vc_ctx_create(xid, NULL))==VC_NOCTX &&
	errno!=EEXIST) {
      perror(ENSC_WRAPPERS_PREFIX "vc_ctx_create()");
      exit(255);
    }

    if (rc!=VC_NOCTX) {
      char			buf[sizeof(xid_t)*3 + 128];
      size_t			l;
      struct vc_ctx_caps	caps;
      struct vc_ctx_flags	flags;
      
      strcpy(buf, "rpm-fake.so #");
      l = utilvserver_fmt_uint(buf+sizeof("rpm-fake.so #")-1, getppid());
      Evc_set_vhi_name(rc, vcVHI_CONTEXT, buf, sizeof("rpm-fake.so #")+l-1);

      caps.ccaps =  0ull;
      caps.cmask = ~0ull;
      caps.bcaps = ~vc_get_insecurebcaps();
      caps.bmask = ~0ull;
      Evc_set_ccaps(rc, &caps);

      flags.flagword = 0;
      flags.mask = VC_VXF_SC_HELPER;
      Evc_set_cflags(rc, &flags);
      
	// context will be activated later...

      xid = rc;
      res = true;
      ctx_created = true;
    }
  }

  if (xid==VC_DYNAMIC_XID)
    *xid_str = 0;
  else {
    char		buf[sizeof(xid_t)*3 + 2];
    size_t		l;
    
    l        = utilvserver_fmt_uint(buf, xid); buf[l] = '\0';
    *xid_str = strdup(buf);
  }
 
  Ewrite(3, &xid, sizeof xid);
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
      perror(ENSC_WRAPPERS_PREFIX "error while initializing pw-socket");
      exit(255);
    }
  }
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
      perror(ENSC_WRAPPERS_PREFIX "failed to create/initialize resolver-socket or pipe");
      exit(255);
    }

    pid = fork();
    if (pid==-1) {
      perror(ENSC_WRAPPERS_PREFIX "fork()");
      exit(255);
    }

    if (pid==0) {
      char const	*args[20];
      char const	**ptr  = args;
      char const 	*env[] = { "HOME=/", "PATH=/bin:/usr/bin", 0 };
      char const	*xid_str;
      char		flag_str[ sizeof(flags)*3 + 2];
      char		caps_str[ sizeof(caps)*3  + 2];

      clearEnv();
      
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

      flag_str[utilvserver_fmt_uint(flag_str, flags)] = '\0';
      caps_str[utilvserver_fmt_uint(caps_str, caps)]  = '\0';

      *ptr++ = resolver;
      *ptr++ = "-F"; *ptr++ = flag_str;
      *ptr++ = "-C"; *ptr++ = caps_str;
      if (root)  { *ptr++ = "-r"; *ptr++ = ".";   }
      if (uid)   { *ptr++ = "-u"; *ptr++ = uid;   }
      if (gid)   { *ptr++ = "-g"; *ptr++ = gid;   }

      if (root) Echdir(root);

      if (setupContext(ctx, &xid_str)) { *ptr++ = "-s"; }
      else if (xid_str)                { *ptr++ = "-c"; *ptr++ = xid_str; }
      
      *ptr++ = 0;
      execve(resolver, (char **)args, (char **)env);
      perror(ENSC_WRAPPERS_PREFIX "failed to exec resolver");
      exit(255);
    }
    else {
      uint8_t		c;

      close(res_sock[1]);
      close(sync_pipe[1]);
      pw_sock   = res_sock[0];
      sync_sock = sync_pipe[0];

      if (read(sync_sock, &ctx, sizeof ctx)!=sizeof(ctx) ||
	  read(sync_sock, &c, 1)!=1 ||
	  write(pw_sock, ".", 1)!=1 ||
	  read(pw_sock, &c,   1)!=1 ||
	  c!='.') {
	WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "failed to initialize communication with resolver\n");
	exit(255);
      }

      if (wait4(pid, 0, WNOHANG,0)==-1) {
	WRITE_MSG(2, ENSC_WRAPPERS_PREFIX" unexpected initialization-error of resolver\n");
	exit(255);
      }
    }
  }
}
#endif

static void
reduceCapabilities()
{
  struct __user_cap_header_struct header;
  struct __user_cap_data_struct user;
  
  header.version = _LINUX_CAPABILITY_VERSION;
  header.pid     = 0;

  if (capget(&header, &user)==-1) {
    perror("capget()");
    exit(wrapper_exit_code);
  }

  user.effective   &= ~(1<<CAP_MKNOD);
  user.permitted   &= ~(1<<CAP_MKNOD);
  user.inheritable &= ~(1<<CAP_MKNOD);

  if (capset(&header, &user)==-1) {
    perror("capset()");
    exit(wrapper_exit_code);
  }
}

static void
initEnvironment()
{
  int		syscall_rev;
  int		syscall_nr;
  
  if (is_initialized) return;

  syscall_rev = getDefaultEnv("RPM_FAKE_S_CONTEXT_REV", 0);
  syscall_nr  = getDefaultEnv("RPM_FAKE_S_CONTEXT_NR",  273);
  
#ifdef VC_ENABLE_API_LEGACY
  {
    extern void vc_init_internal_legacy(int ctx_rev, int ctx_number,
					int ipv4_rev, int ipv4_number);
  
    vc_init_internal_legacy(syscall_rev, syscall_nr, 3, 274);
  }
#endif

  ctx       = getDefaultEnv("RPM_FAKE_CTX",  VC_DYNAMIC_XID);
  caps      = getDefaultEnv("RPM_FAKE_CAP",  ~0x3404040f);
  flags     = getDefaultEnv("RPM_FAKE_FLAGS", 0);
  root      = getenv("RPM_FAKE_CHROOT");
  mnts      = getenv("RPM_FAKE_NAMESPACE_MOUNTS");
  if (mnts && *mnts) mnts = strdup(mnts);
  else               mnts = 0;

#if DEBUG
  if (isDbgLevel(DBG_VERBOSE1))
    dprintf(2, "ctx=%u, caps=%016x, flags=%016x,\nroot='%s',\nmnts='%s'\n",
	    ctx, caps, flags, root, mnts);
#endif
  
  is_initialized = true;
}

static void
initSymbols()
{
    //INIT(RTLD_NEXT, rpm_execcon);
    //INIT(RTLD_NEXT, execv);
  INIT(RTLD_NEXT, getgrnam);
  INIT(RTLD_NEXT, getpwnam);
  INIT(RTLD_NEXT, endpwent);
  INIT(RTLD_NEXT, endgrent);
}

void
initRPMFake()
{
  if (getenv("RPM_FAKE_VERSION")) showVersion();
  if (getenv("RPM_FAKE_HELP"))    showHelp();
  
  debug_level = getDefaultEnv("RPM_FAKE_DEBUG", 0);

  if (isDbgLevel(DBG_INIT)) WRITE_MSG(2, ">>>>> initRPMFake <<<<<\n");
  
  reduceCapabilities();
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
    if (read(sync_sock, &c, 1)!=1) { /*...*/ }
    if (write(pw_sock, "Q", 1)!=1) { /*...*/ }
    if (ctx_created) {
      if (vc_isSupported(vcFEATURE_VWAIT)) {
	if (vc_wait_exit(ctx)==-1) { /*...*/ }
      }
      else {
	/* this can race */
	if (read(sync_sock, &c, 1)!=0) { /*...*/}
      }
    }
  }
}


  //============   the worker part   ===========


static bool
doPwStringRequest(uint32_t *result, char style, char const *name)
{
  uint32_t	len = strlen(name);
  uint8_t	code;
  uint8_t	c;

  return (TEMP_FAILURE_RETRY(read (sync_sock, &c, 1))==1 &&
	  TEMP_FAILURE_RETRY(write(pw_sock, &style, 1))==1 &&
	  TEMP_FAILURE_RETRY(write(pw_sock, &len,   sizeof len))==sizeof(len) &&
	  TEMP_FAILURE_RETRY(write(pw_sock, name,   len))==(ssize_t)(len) &&
	  TEMP_FAILURE_RETRY(read (pw_sock, &code,  sizeof code))==sizeof(code) &&
	  TEMP_FAILURE_RETRY(read (pw_sock, result, sizeof *result))==sizeof(*result) &&
	  code!=0);
}

struct passwd *
getpwnam(const char * name)
{
  if (pw_sock==-1) return getpwnam_func(name);
  else {
    uint32_t			id;
    static struct passwd	res = {
      .pw_passwd = "*",
      .pw_gid    = -1,
      .pw_gecos  = "",
      .pw_dir    = "/",
      .pw_shell  = "/bin/false"
    };

    res.pw_name = (char *)(name);
    if (!doPwStringRequest(&id, 'P', name)) return 0;
    res.pw_uid = id;
    
    return &res;
  }
}

struct group *
getgrnam(const char * name)
{
  if (pw_sock==-1) return getgrnam_func(name);
  else {
    uint32_t			id;
    static struct group		res = {
      .gr_passwd = "*",
      .gr_mem    = 0
    };

    res.gr_name = (char *)(name);
    if (!doPwStringRequest(&id, 'G', name)) return 0;
    res.gr_gid = id;

    return &res;
  }
}

void
endgrent()
{
  if (pw_sock==-1) endgrent_func();
  TEMP_FAILURE_RETRY(write(pw_sock, "Cg", 2));
}

void
endpwent()
{
  if (pw_sock==-1) endpwent_func();
  TEMP_FAILURE_RETRY(write(pw_sock, "Cp", 2));
}


static int
execvWorker(char const *path, char * const argv[], char * const envp[])
{
  int		res = -1;

  if (vc_isSupported(vcFEATURE_MIGRATE))
    res = vc_ctx_migrate(ctx, 0);
  else {
#ifdef VC_ENABLE_API_COMPAT  
    res = vc_new_s_context(ctx,caps,flags);
#else
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "can not change context: migrate kernel feature missing and 'compat' API disabled\n");
#endif
  }

  clearEnv();    
    
  if (res!=-1)
    res=execve(path, argv, envp);

  return res;
}

static int
removeNamespaceMounts(char const *mnts)
{
  char			buf[strlen(mnts)+1], *ptr;

  strcpy(buf, mnts);
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

  return 0;
}

static int
execv_main(char const *path, char * const argv[], char * const envp[])
{
  int		rc = 0;

  if (sys_unshare(CLONE_NEWNS)==-1) {
    perror("unshare()");
    return -1;
  }

  if (mnts)
    rc = removeNamespaceMounts(mnts);

  if (rc!=0)
    return rc;
  
  return execvWorker(path, argv, envp);
}

int
execv(char const *path, char * const argv[])
{
  extern char **environ;

  if (isDbgLevel(DBG_EXECV)) {
    WRITE_MSG(2, "execv('");
    WRITE_STR(2, path);
    WRITE_MSG(2, "', ...)\n");
  }

  return execv_main(path, argv, environ);
}

int
rpm_execcon(unsigned int UNUSED verified,
	    const char *filename,
	    char *const argv[], char *const envp[])
{
  if (isDbgLevel(DBG_EXECV)) {
    WRITE_MSG(2, "rpm_execcon(..., '");
    WRITE_STR(2, filename);
    WRITE_MSG(2, "', ...)\n");
  }

  return execv_main(filename, argv, envp);
}

int
is_selinux_enabled()
{
  return 0;
}
