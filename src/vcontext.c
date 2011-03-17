// $Id$    --*- c -*--

// Copyright (C) 2004-2006 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#include "compat-pivot_root.h"
#include "lib/internal.h"
#include "lib_internal/jail.h"
#include "lib_internal/sys_personality.h"
#include "lib_internal/sys_unshare.h"

#include <vserver.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/mount.h>

#include <linux/personality.h>

#define ENSC_WRAPPERS_PREFIX	"vcontext: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_SOCKET	1
#define ENSC_WRAPPERS_IOSOCK	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_XID			0x4000
#define CMD_CREATE		0x4001
#define CMD_MIGRATE		0x4003
#define CMD_INITPID		0x4002
#define CMD_DISCONNECT		0x4004
#define CMD_UID			0x4005
#define CMD_CHROOT		0x4006
#define CMD_SILENT		0x4007
#define CMD_SYNCSOCK		0x4008
#define CMD_SYNCMSG		0x4009
#define CMD_MIGRATESELF		0x400a
#define CMD_ENDSETUP		0x400b
#define CMD_SILENTEXIST		0x400c
#define CMD_NAMESPACE		0x400d
#define CMD_PERSTYPE		0x400e
#define CMD_PERSFLAG		0x400f
#define CMD_VLOGIN		0x4010
#define CMD_PIVOT_ROOT		0x4011


#ifndef MNT_DETACH
#define MNT_DETACH		0x0002
#endif


struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,       0, CMD_HELP },
  { "version",    no_argument,       0, CMD_VERSION },
  { "ctx",        required_argument, 0, CMD_XID },
  { "xid",        required_argument, 0, CMD_XID },
  { "create",     no_argument,       0, CMD_CREATE },
  { "migrate",    no_argument,       0, CMD_MIGRATE },
  { "migrate-self", no_argument,       	0, CMD_MIGRATESELF },
  { "initpid",      no_argument,       	0, CMD_INITPID },
  { "endsetup",     no_argument,        0, CMD_ENDSETUP },
  { "disconnect",   no_argument,	0, CMD_DISCONNECT },
  { "silent",       no_argument,       	0, CMD_SILENT },
  { "silentexist",  no_argument,       	0, CMD_SILENTEXIST },
  { "uid",          required_argument,  0, CMD_UID },
  { "chroot",       no_argument,       	0, CMD_CHROOT },
  { "namespace",    no_argument,       	0, CMD_NAMESPACE },
  { "syncsock",     required_argument, 	0, CMD_SYNCSOCK },
  { "syncmsg",      required_argument, 	0, CMD_SYNCMSG },
  { "personality-type",  required_argument, 0, CMD_PERSTYPE },
  { "personality-flags", required_argument, 0, CMD_PERSFLAG },
  { "vlogin",       no_argument,        0, CMD_VLOGIN },
  { "pivot-root",   no_argument,        0, CMD_PIVOT_ROOT },
#if 1
  { "fakeinit",     no_argument,       	0, CMD_INITPID },	// compatibility
#endif
  { 0,0,0,0 },
};

struct Arguments {
    bool		do_create;
    bool		do_migrate;
    bool		do_migrateself;
    bool		do_disconnect;
    bool		do_endsetup;
    bool		is_initpid;
    bool		is_silentexist;
    bool		set_namespace;
    bool		do_vlogin;
    uint_least32_t	personality_flags;
    uint_least32_t	personality_type;
    int			verbosity;
    bool		do_chroot;
    bool		do_pivot_root;
    char const *	uid;
    xid_t		xid;
    char const *	sync_sock;
    char const *	sync_msg;
};

int		wrapper_exit_code = 255;

void do_vlogin(int argc, char *argv[], int ind);

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n    ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --create [--xid <xid>] <opts>* [--] <program> <args>*\n    ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [(--migrate --xid <xid>)|--migrate-self]  <opts>* [--] <program> <args>*\n"
	    "\n"
	    "<opts> can be:\n"
	    "    --chroot	 ...  chroot into current directory\n"
	    "    --namespace     ...  execute namespace management operations\n"
	    "    --uid <uid>     ...  change uid\n"
	    "    --initpid       ...  set current process as general process reaper\n"
	    "                         for ctx (possible for --migrate only)\n"
	    "    --endsetup      ...  clear the setup flag; usefully for migrate only\n"
	    "    --disconnect    ...  start program in background\n"
	    "    --personality-type <type>\n"
	    "                    ...  execute <program> in the given execution domain\n"
	    "    --personality-flags <flags>+\n"
	    "                    ...  set special flags for the given execution domain\n"
	    "    --silent        ...  be silent\n"
	    "    --silentexist   ...  be silent when context exists already; usefully\n"
	    "                         for '--create' only\n"
	    "    --syncsock <file-name>\n"
	    "                    ...  before executing the program, send a message\n"
	    "                         to the socket and wait until it closes.\n"
	    "                         <file-name> must be a SOCK_STREAM unix socket\n"
	    "    --syncmsg <message>\n"
	    "                    ...  use <message> as synchronization message; by\n"
	    "                         default, 'ok' will be used\n"
	    "    --vlogin        ...  enable terminal proxy\n"
	    "\n"
	    "'vcontext --create' exits with code 254 iff the context exists already.\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vcontext " VERSION " -- manages the creation of security contexts\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004-2006 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

#include "context-sync.hc"

static inline ALWAYSINLINE void
tellContext(xid_t ctx, bool do_it)
{
  char		buf[sizeof(xid_t)*3+2];
  size_t	l;

  if (!do_it) return;

  l = utilvserver_fmt_long(buf,ctx);

  WRITE_MSG(1, "New security context is ");
  Vwrite   (1, buf, l);
  WRITE_MSG(1, "\n");
}

static int
connectExternalSync(char const *filename)
{
  int			fd;
  struct sockaddr_un	addr;

  if (filename==0) return -1;

  ENSC_INIT_UNIX_SOCK(addr, filename);

  fd = Esocket(PF_UNIX, SOCK_STREAM, 0);
  Econnect(fd, &addr, sizeof(addr));

  return fd;
}

static void
setFlags(struct Arguments const *args, xid_t xid)
{
  struct vc_ctx_flags	flags = { 0,0 };

  if (args->is_initpid)
    flags.mask |=  VC_VXF_STATE_INIT;

  if (args->do_endsetup)
    flags.mask |=  VC_VXF_STATE_SETUP;

  if (flags.mask!=0) {
    DPRINTF("set_flags: mask=%08llx, flag=%08llx\n", flags.mask, flags.flagword);
    Evc_set_cflags(xid, &flags);
  }
}

static void
doExternalSync(int fd, char const *msg)
{
  char		c;

  if (fd==-1) return;

  if (msg) EsendAll(fd, msg, strlen(msg));
  Eshutdown(fd, SHUT_WR);

  if (TEMP_FAILURE_RETRY(recv(fd, &c, 1, MSG_NOSIGNAL))!=0) {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "unexpected external synchronization event\n");
    exit(wrapper_exit_code);
  }

  Eclose(fd);
}

static inline ALWAYSINLINE int
doit(struct Arguments const *args, int argc, char *argv[])
{
  int			p[2][2];
  pid_t			pid = initSync(p, args->do_disconnect);

  if (pid==0) {
    xid_t			xid;
    int				ext_sync_fd = connectExternalSync(args->sync_sock);

    doSyncStage0(p, args->do_disconnect);

    if (args->do_create) {
      xid = vc_ctx_create(args->xid, NULL);
      if (xid==VC_NOCTX) {
	switch (errno) {
	  case EEXIST	:
	    if (!args->is_silentexist)
	      perror(ENSC_WRAPPERS_PREFIX "vc_ctx_create()");
	    return 254;
	  default	:
	    perror(ENSC_WRAPPERS_PREFIX "vc_ctx_create()");
	    return wrapper_exit_code;
	}
      }
      tellContext(xid, args->verbosity>=1);
    }
    else
      xid = args->xid;

    if (args->do_chroot) {
      Echroot(".");
      if (args->set_namespace) {
	if (args->do_migrateself)  Evc_set_namespace(xid, CLONE_NEWNS|CLONE_FS, 0);
	else if (args->do_migrate) Evc_enter_namespace(xid, CLONE_NEWNS|CLONE_FS, 0);
      }
    }
    else if (args->do_pivot_root) {
      if (vc_enter_namespace(xid, CLONE_NEWNS | CLONE_FS, 1) == -1) {
	bool existed = false;
	if (sys_unshare(CLONE_NEWNS) == -1) {
	  perror(ENSC_WRAPPERS_PREFIX "unshare(NEWNS)");
	  return wrapper_exit_code;
	}
	if (mkdir("./.oldroot", 0700) == -1) {
	  if (errno == EEXIST)
	    existed = true;
	  else {
	    perror(ENSC_WRAPPERS_PREFIX "mkdir()");
	    return wrapper_exit_code;
	  }
	}
	if (pivot_root(".", "./.oldroot") == -1) {
	  perror(ENSC_WRAPPERS_PREFIX "pivot_root()");
	  return wrapper_exit_code;
	}
	if (umount2("/.oldroot", MNT_DETACH) == -1) {
	  perror(ENSC_WRAPPERS_PREFIX "umount2()");
	  return wrapper_exit_code;
	}
	if (!existed && rmdir("/.oldroot") == -1) {
	  perror(ENSC_WRAPPERS_PREFIX "rmdir()");
	  return wrapper_exit_code;
	}
	Evc_set_namespace(xid, CLONE_NEWNS | CLONE_FS, 1);
      }
    }

    setFlags(args, xid);

    if (args->do_migrate && !args->do_migrateself)
      Evc_ctx_migrate(xid, 0);

    if (args->uid != NULL) {
      uid_t uid = 0;
      unsigned long tmp;

      if (!isNumberUnsigned(args->uid, &tmp, false)) {
#ifdef __dietlibc__
	struct passwd *pw;
	pw = getpwnam(args->uid);
	if (pw == NULL) {
	  WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Username '");
	  WRITE_STR(2, args->uid);
	  WRITE_MSG(2, "' does not exist\n");
	  return wrapper_exit_code;
	}
	uid = pw->pw_uid;
	Einitgroups(args->uid, pw->pw_gid);
	Esetgid(pw->pw_gid);
#else
	WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Uid '");
	WRITE_STR(2, args->uid);
	WRITE_MSG(2, "' is not a number\n");
	return wrapper_exit_code;
#endif
      }
      else
	uid = (uid_t) tmp;

      Esetuid((uid_t) uid);
      if (getuid()!=uid) {
	WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Something went wrong while changing the UID\n");
	exit(wrapper_exit_code);
      }
    }

    if (args->personality_type!=VC_BAD_PERSONALITY &&
	sys_personality(args->personality_type | args->personality_flags)==-1) {
      perror(ENSC_WRAPPERS_PREFIX "personality()");
      exit(wrapper_exit_code);
    }

    doExternalSync(ext_sync_fd, args->sync_msg);
    doSyncStage1(p, args->do_disconnect);
    DPRINTF("doit: pid=%u, ppid=%u\n", getpid(), getppid());

    if (!args->do_vlogin)
      execvp (argv[optind],argv+optind);
    else
      do_vlogin(argc, argv, optind);
    doSyncStage2(p, args->do_disconnect);

    PERROR_Q(ENSC_WRAPPERS_PREFIX "execvp", argv[optind]);
    exit(wrapper_exit_code);
  }

  assert(args->do_disconnect);

  waitOnSync(pid, p, args->xid!=VC_DYNAMIC_XID && args->do_migrate);
  return EXIT_SUCCESS;
}

static uint_least32_t
parsePersonalityType(char const *str)
{
  uint_least32_t	res = vc_str2personalitytype(str, 0);
  if (res==VC_BAD_PERSONALITY) {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "bad personality type\n");
    exit(wrapper_exit_code);
  }

  return res;
}

static uint_least32_t
parsePersonalityFlags(char const *str)
{
  struct vc_err_listparser	err;
  uint_least32_t		res;

  if (vc_list2personalityflag(str, 0, &res, &err)==-1) {
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "bad personality flag '");
    Vwrite(2, err.ptr, err.len);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }

  return res;
}

int main (int argc, char *argv[])
{
  struct Arguments		args = {
    .do_create         = false,
    .do_migrate        = false,
    .do_migrateself    = false,
    .do_disconnect     = false,
    .do_endsetup       = false,
    .do_vlogin         = false,
    .is_initpid        = false,
    .is_silentexist    = false,
    .set_namespace     = false,
    .verbosity         = 1,
    .uid               = NULL,
    .xid               = VC_DYNAMIC_XID,
    .personality_type  = VC_BAD_PERSONALITY,
    .personality_flags = 0,
    .sync_msg          = "ok",
  };

  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_CREATE		:  args.do_create      = true;   break;
      case CMD_MIGRATE		:  args.do_migrate     = true;   break;
      case CMD_DISCONNECT	:  args.do_disconnect  = true;   break;
      case CMD_ENDSETUP		:  args.do_endsetup    = true;   break;
      case CMD_VLOGIN		:  args.do_vlogin      = true;   break;
      case CMD_INITPID		:  args.is_initpid     = true;   break;
      case CMD_CHROOT		:  args.do_chroot      = true;   break;
      case CMD_PIVOT_ROOT	:  args.do_pivot_root  = true;   break;
      case CMD_NAMESPACE	:  args.set_namespace  = true;   break;
      case CMD_SILENTEXIST	:  args.is_silentexist = true;   break;
      case CMD_SYNCSOCK		:  args.sync_sock      = optarg; break;
      case CMD_SYNCMSG		:  args.sync_msg       = optarg; break;
      case CMD_UID		:  args.uid            = optarg; break;
      case CMD_XID		:  args.xid = Evc_xidopt2xid(optarg,true); break;
      case CMD_SILENT		:  --args.verbosity; break;
      case CMD_PERSTYPE		:
	args.personality_type   = parsePersonalityType(optarg);
	break;
      case CMD_PERSFLAG		:
	args.personality_flags |= parsePersonalityFlags(optarg);
	break;
      case CMD_MIGRATESELF	:
	args.do_migrate     = true;
	args.do_migrateself = true;
	break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return wrapper_exit_code;
	break;
    }
  }

  signal(SIGCHLD, SIG_DFL);

  if (args.do_migrateself)
    args.xid = Evc_get_task_xid(0);

  if (!args.do_create && !args.do_migrate)
    WRITE_MSG(2, "Neither '--create' nor '--migrate' specified; try '--help' for more information\n");
  else if (args.do_create  &&  args.do_migrate)
    WRITE_MSG(2, "Can not specify '--create' and '--migrate' at the same time; try '--help' for more information\n");
  else if (!args.do_migrate && args.is_initpid)
    WRITE_MSG(2, "'--initpid' is possible in combination with '--migrate' only\n");
  else if (!args.do_create && args.xid==VC_DYNAMIC_XID)
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Can not migrate to an unknown context\n");
  else if (optind>=argc)
    WRITE_MSG(2, "No command given; use '--help' for more information.\n");
  else
    return doit(&args, argc, argv);

  return wrapper_exit_code;
}
