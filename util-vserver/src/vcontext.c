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

#include "util.h"
#include "lib/internal.h"
#include "lib_internal/jail.h"

#include "linuxcaps.h"

#include <vserver.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>

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
#define CMD_FAKEINIT		0x4002
#define CMD_DISCONNECT		0x4004
#define CMD_UID			0x4005
#define CMD_CHROOT		0x4006
#define CMD_SILENT		0x4007
#define CMD_SYNCSOCK		0x4008
#define CMD_SYNCMSG		0x4009
#define CMD_MIGRATESELF		0x400a
#define CMD_ENDSETUP		0x400b
#define CMD_SILENTEXIST		0x400c

struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,       0, CMD_HELP },
  { "version",    no_argument,       0, CMD_VERSION },
  { "ctx",        required_argument, 0, CMD_XID },
  { "xid",        required_argument, 0, CMD_XID },
  { "create",     no_argument,       0, CMD_CREATE },
  { "migrate",    no_argument,       0, CMD_MIGRATE },
  { "migrate-self", no_argument,       	0, CMD_MIGRATESELF },
  { "fakeinit",     no_argument,       	0, CMD_FAKEINIT },
  { "endsetup",     no_argument,        0, CMD_ENDSETUP },
  { "disconnect",   no_argument,	0, CMD_DISCONNECT },
  { "silent",       no_argument,       	0, CMD_SILENT },
  { "silentexist",  no_argument,       	0, CMD_SILENTEXIST },
  { "uid",          required_argument,  0, CMD_UID },
  { "chroot",       no_argument,       	0, CMD_CHROOT },
  { "syncsock",     required_argument, 	0, CMD_SYNCSOCK },
  { "syncmsg",      required_argument, 	0, CMD_SYNCMSG },
  { 0,0,0,0 },
};

struct Arguments {
    bool		do_create;
    bool		do_migrate;
    bool		do_migrateself;
    bool		do_disconnect;
    bool		do_endsetup;
    bool		is_fakeinit;
    bool		is_silentexist;
    int			verbosity;
    bool		do_chroot;
    uid_t		uid;
    xid_t		xid;
    char const *	sync_sock;
    char const *	sync_msg;
};

int		wrapper_exit_code = 255;

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
	    "    --uid <uid>     ...  change uid\n"
	    "    --fakeinit      ...  set current process as general process reaper\n"
	    "                         for ctx (possible for --migrate only)\n"
	    "    --endsetup      ...  clear the setup flag; usefully for migrate only\n"
	    "    --disconnect    ...  start program in background\n"
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
	    "Copyright (C) 2004 Enrico Scholz\n"
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
  write(1, buf, l);
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

  if (args->is_fakeinit)
    flags.mask |= VC_VXF_STATE_INIT;

  if (args->do_endsetup)
    flags.mask |= VC_VXF_STATE_SETUP;

  if (flags.mask!=0) 
    Evc_set_flags(xid, &flags);
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
    exit(255);
  }

  Eclose(fd);
}

static inline ALWAYSINLINE int
doit(struct Arguments const *args, char *argv[])
{
  int			p[2][2];
  pid_t			pid = initSync(p, args->do_disconnect);
  
  if (pid==0) {
    xid_t			xid;
    int				ext_sync_fd = connectExternalSync(args->sync_sock);

    doSyncStage0(p, args->do_disconnect);
    
    if (args->do_create) {
      xid = vc_create_context(args->xid);
      if (xid==VC_NOCTX) {
	switch (errno) {
	  case EEXIST	:
	    if (!args->is_silentexist)
	      perror(ENSC_WRAPPERS_PREFIX "vc_create_context()");
	    return 254;
	  default	:
	    perror(ENSC_WRAPPERS_PREFIX "vc_create_context()");
	    return wrapper_exit_code;
	}
      }
      tellContext(xid, args->verbosity>=1);
    }
    else
      xid = args->xid;

    if (args->do_chroot)
      Echroot(".");

    setFlags(args, xid);

    if (args->do_migrate && !args->do_migrateself)
      Evc_migrate_context(xid);

    if (args->uid!=(uid_t)(-1) && getuid()!=args->uid) {
      Esetuid(args->uid);
      if (getuid()!=args->uid) {
	WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Something went wrong while changing the UID\n");
	exit(255);
      }
    }
    
    doExternalSync(ext_sync_fd, args->sync_msg);
    doSyncStage1(p, args->do_disconnect);
    execvp (argv[optind],argv+optind);
    doSyncStage2(p, args->do_disconnect);

    PERROR_Q(ENSC_WRAPPERS_PREFIX "execvp", argv[optind]);
    exit(255);
  }

  assert(args->do_disconnect);
    
  waitOnSync(pid, p, args->xid!=VC_DYNAMIC_XID && args->do_migrate);
  return EXIT_SUCCESS;
}

int main (int argc, char *argv[])
{
  struct Arguments		args = {
    .do_create      = false,
    .do_migrate     = false,
    .do_migrateself = false,
    .do_disconnect  = false,
    .do_endsetup    = false,
    .is_fakeinit    = false,
    .is_silentexist = false,
    .verbosity      = 1,
    .uid            = -1,
    .xid            = VC_DYNAMIC_XID,
    .sync_msg       = "ok",
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
      case CMD_FAKEINIT		:  args.is_fakeinit    = true;   break;
      case CMD_CHROOT		:  args.do_chroot      = true;   break;
      case CMD_SILENTEXIST	:  args.is_silentexist = true;   break;
      case CMD_SYNCSOCK		:  args.sync_sock      = optarg; break;
      case CMD_SYNCMSG		:  args.sync_msg       = optarg; break;
      case CMD_XID		:  args.xid            = atol(optarg); break;
      case CMD_UID		:  args.uid            = atol(optarg); break;
      case CMD_SILENT		:  --args.verbosity; break;
      case CMD_MIGRATESELF	:
	args.do_migrate     = true;
	args.do_migrateself = true;
	break;

      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return 255;
	break;
    }
  }

  if (args.do_migrateself)
    args.xid = Evc_get_task_xid(0);
  
  if (!args.do_create && !args.do_migrate)
    WRITE_MSG(2, "Neither '--create' nor '--migrate specified; try '--help' for more information\n");
  else if (args.do_create  &&  args.do_migrate)
    WRITE_MSG(2, "Can not specify '--create' and '--migrate' at the same time; try '--help' for more information\n");
  else if (!args.do_migrate && args.is_fakeinit)
    WRITE_MSG(2, "'--fakeinit' is possible in combination with '--migrate' only\n");
  else if (!args.do_create && args.xid==VC_DYNAMIC_XID)
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX "Can not migrate to an unknown context\n");
  else if (optind>=argc)
    WRITE_MSG(2, "No command given; use '--help' for more information.\n");
  else
    return doit(&args, argv);

  return 255;
}
