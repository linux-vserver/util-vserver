// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on reducecap.cc by Jacques Gelinas
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util.h"
#include "vserver.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/param.h>

#include "linuxcaps.h"

#define ENSC_WRAPPERS_PREFIX	"reducecap: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>


#define CMD_HELP	0x1000
#define CMD_VERSION	0x1001

#define CMD_SHOW	0x2000
#define CMD_PID		0x2001

#define CMD_CAP		0x4000
#define CMD_FLAG	0x4004
#define CMD_SECURE	0x4006

#ifdef VC_ENABLE_API_LEGACY
#  define CMD_OBSOLETE_CHOWN			0x8000
#  define CMD_OBSOLETE_DAC_OVERRIDE		0x8001
#  define CMD_OBSOLETE_DAC_READ_SEARCH		0x8002
#  define CMD_OBSOLETE_FOWNER			0x8003
#  define CMD_OBSOLETE_FSETID			0x8004
#  define CMD_OBSOLETE_KILL			0x8005
#  define CMD_OBSOLETE_SETGID			0x8006
#  define CMD_OBSOLETE_SETUID			0x8007
#  define CMD_OBSOLETE_SETPCAP			0x8008
#  define CMD_OBSOLETE_SYS_TTY_CONFIG		0x8009
#  define CMD_OBSOLETE_LEASE			0x800a
#  define CMD_OBSOLETE_SYS_CHROOT		0x800b
#  define CMD_OBSOLETE_X_LINUX_IMMUTABLE	0x800c
#  define CMD_OBSOLETE_X_NET_BIND_SERVICE	0x800d
#  define CMD_OBSOLETE_X_NET_BROADCAST		0x800e
#  define CMD_OBSOLETE_X_NET_ADMIN		0x800f
#  define CMD_OBSOLETE_X_NET_RAW		0x8010
#  define CMD_OBSOLETE_X_IPC_LOCK		0x8011
#  define CMD_OBSOLETE_X_IPC_OWNER		0x8012
#  define CMD_OBSOLETE_X_SYS_MODULE		0x8013
#  define CMD_OBSOLETE_X_SYS_RAWIO		0x8014
#  define CMD_OBSOLETE_X_SYS_PACCT		0x8015
#  define CMD_OBSOLETE_X_SYS_ADMIN		0x8016
#  define CMD_OBSOLETE_X_SYS_BOOT		0x8017
#  define CMD_OBSOLETE_X_SYS_NICE		0x8018
#  define CMD_OBSOLETE_X_SYS_RESOURCE		0x8019
#  define CMD_OBSOLETE_X_SYS_TIME		0x801a
#  define CMD_OBSOLETE_X_MKNOD			0x801b
#  define CMD_OBSOLETE_X_QUOTACTL		0x801c

static char const * const	OBSOLETE_MAPPING[] = {
  // 0                  1                  2                  3
  "CHOWN",           "DAC_OVERRIDE",     "DAC_READ_SEARCH", "FOWNER",
  "FSETID",          "KILL",             "SETGID",          "SETUID",
  "SETPCAP",         "SYS_TTY_CONFIG",   "LEASE",           "SYS_CHROOT",
  "LINUX_IMMUTABLE", "NET_BIND_SERVICE", "NET_BROADCAST",   "NET_ADMIN",
  "NET_RAW",         "IPC_LOCK",         "IPC_OWNER",       "SYS_MODULE",
  "SYS_RAWIO",       "SYS_PACCT",        "SYS_ADMIN",       "SYS_BOOT",
  "SYS_NICE",        "SYS_RESOURCE",     "SYS_TIME",        "MKNOD",
  "QUOTACTL" };
#endif

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "cap",        required_argument,  0, CMD_CAP },
  { "flag",       required_argument,  0, CMD_FLAG },
  { "secure",     no_argument,        0, CMD_SECURE },
  { "show",       no_argument,        0, CMD_SHOW },
  { "pid",        required_argument,  0, CMD_PID },
#ifdef VC_ENABLE_API_LEGACY
  { "CAP_CHOWN",              no_argument,  0, CMD_OBSOLETE_CHOWN },
  { "CAP_DAC_OVERRIDE",       no_argument,  0, CMD_OBSOLETE_DAC_OVERRIDE },
  { "CAP_DAC_READ_SEARCH",    no_argument,  0, CMD_OBSOLETE_DAC_READ_SEARCH },
  { "CAP_FOWNER",             no_argument,  0, CMD_OBSOLETE_FOWNER },
  { "CAP_FSETID",             no_argument,  0, CMD_OBSOLETE_FSETID },
  { "CAP_KILL",               no_argument,  0, CMD_OBSOLETE_KILL },
  { "CAP_SETGID",             no_argument,  0, CMD_OBSOLETE_SETGID },
  { "CAP_SETUID",             no_argument,  0, CMD_OBSOLETE_SETUID },
  { "CAP_SETPCAP",            no_argument,  0, CMD_OBSOLETE_SETPCAP },
  { "CAP_SYS_TTY_CONFIG",     no_argument,  0, CMD_OBSOLETE_SYS_TTY_CONFIG },
  { "CAP_LEASE",              no_argument,  0, CMD_OBSOLETE_LEASE },
  { "CAP_SYS_CHROOT",         no_argument,  0, CMD_OBSOLETE_SYS_CHROOT },
  { "--CAP_LINUX_IMMUTABLE",  no_argument,  0, CMD_OBSOLETE_X_LINUX_IMMUTABLE },
  { "--CAP_NET_BIND_SERVICE", no_argument,  0, CMD_OBSOLETE_X_NET_BIND_SERVICE },
  { "--CAP_NET_BROADCAST",    no_argument,  0, CMD_OBSOLETE_X_NET_BROADCAST },
  { "--CAP_NET_ADMIN",        no_argument,  0, CMD_OBSOLETE_X_NET_ADMIN },
  { "--CAP_NET_RAW",          no_argument,  0, CMD_OBSOLETE_X_NET_RAW },
  { "--CAP_IPC_LOCK",         no_argument,  0, CMD_OBSOLETE_X_IPC_LOCK },
  { "--CAP_IPC_OWNER",        no_argument,  0, CMD_OBSOLETE_X_IPC_OWNER },
  { "--CAP_SYS_MODULE",       no_argument,  0, CMD_OBSOLETE_X_SYS_MODULE },
  { "--CAP_SYS_RAWIO",        no_argument,  0, CMD_OBSOLETE_X_SYS_RAWIO },
  { "--CAP_SYS_PACCT",        no_argument,  0, CMD_OBSOLETE_X_SYS_PACCT },
  { "--CAP_SYS_ADMIN",        no_argument,  0, CMD_OBSOLETE_X_SYS_ADMIN },
  { "--CAP_SYS_BOOT",         no_argument,  0, CMD_OBSOLETE_X_SYS_BOOT },
  { "--CAP_SYS_NICE",         no_argument,  0, CMD_OBSOLETE_X_SYS_NICE },
  { "--CAP_SYS_RESOURCE",     no_argument,  0, CMD_OBSOLETE_X_SYS_RESOURCE },
  { "--CAP_SYS_TIME",         no_argument,  0, CMD_OBSOLETE_X_SYS_TIME },
  { "--CAP_MKNOD",            no_argument,  0, CMD_OBSOLETE_X_MKNOD },
  { "--CAP_QUOTACTL",         no_argument,  0, CMD_OBSOLETE_X_QUOTACTL }, 
#endif
  { 0,0,0,0 }
};

int wrapper_exit_code	= 255;

extern int capget (struct __user_cap_header_struct *, struct __user_cap_data_struct *);
extern int capset (struct __user_cap_header_struct *, struct __user_cap_data_struct *);

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--show] [--secure] [--flag <flag>] [--cap <capability>] [--] <cmd> <args>*\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --show [--pid <pid>]\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "reducecap " VERSION " -- starts programs with reduced capabilities\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
printReducecap(struct __user_cap_data_struct *user)
{
  int i;
  WRITE_MSG(1, "            Capability Effective  Permitted  Inheritable\n");

  for (i=0;; ++i) {
    size_t const	len  = 23 + 10*2 + 4+2;
    char const *	text = vc_cap2text(i);
    int			bit  = 1<<i;
    size_t		l;
    char		buf[len];
    if (text==0) break;

    memset(buf, ' ', sizeof buf);
    buf[len-1] = '\n';
    l = MIN(strlen(text), 22);
    memcpy(buf, text, l);
    buf[23 + 10*0 + 4] = (user->effective   & bit) ? 'X' : ' ';
    buf[23 + 10*1 + 4] = (user->permitted   & bit) ? 'X' : ' ';
    buf[23 + 10*2 + 4] = (user->inheritable & bit) ? 'X' : ' ';
    write(1, buf, len);
  }
}

static void
show(pid_t pid)
{
  struct __user_cap_header_struct header;
  struct __user_cap_data_struct user;
  header.version = _LINUX_CAPABILITY_VERSION;
  header.pid     = pid;
  if (capget(&header,&user)==-1){
    perror ("reducecap: capget()");
    exit(wrapper_exit_code);
  }
  
  printReducecap(&user);
}

static uint32_t
getCap(char const *cap)
{
  int		bit = vc_text2cap(cap);
  if (bit!=0) {
    WRITE_MSG(2, "Unknown capability '");
    WRITE_STR(2, optarg);
    WRITE_MSG(2, "'; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  return (1<<bit);
}

int main (int argc, char *argv[])
{
  uint32_t		remove  = 0;
  bool			do_show = false;
  uint32_t		flags   = 0;
  pid_t			pid     = 0;
#ifdef VC_ENABLE_API_LEGACY
  bool			show_obsolete_warning = true;
#endif    

  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

#ifdef VC_ENABLE_API_LEGACY
    if (c>=CMD_OBSOLETE_CHOWN && c<=CMD_OBSOLETE_X_QUOTACTL) {
      if (show_obsolete_warning) {
	WRITE_MSG(2, "reducecap: warning, obsolete CLI used\n");
	show_obsolete_warning = false;
      }

      remove = getCap(OBSOLETE_MAPPING[c-CMD_OBSOLETE_CHOWN]);
      continue;
    }
#endif    
    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_SECURE		:  remove  = vc_get_insecurecaps(); break;
      case CMD_SHOW		:  do_show = true;  break; 
      case CMD_PID		:  pid     = atoi(optarg);   break;
      case CMD_CAP		:  remove  = getCap(optarg); break;
      case CMD_FLAG		: {
	struct vc_err_listparser	err;
	
	flags = vc_list2cflag_compat(optarg, 0, &err);
	if (err.ptr!=0) {
	  WRITE_MSG(2, "Unknown flag '");
	  write(2, err.ptr, err.len);
	  WRITE_MSG(2, "'\n");
	  exit(wrapper_exit_code);
	}
	break;
      }
    }
  }

  if (!do_show && optind==argc) {
    WRITE_MSG(2, "No command given; use '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  if (!do_show && pid!=0) {
    WRITE_MSG(2, "A pid can be specified in '--show' mode only; use '--help' for more information\n");
    exit(wrapper_exit_code);
  }  

  if (do_show && optind==argc)
    show(pid);
  else {
    Evc_new_s_context(VC_SAMECTX, remove, flags);
    if (do_show) show(pid);

    WRITE_MSG(2, "Executing\n");
    Eexecvp(argv[optind], argv+optind);
  }

  return EXIT_SUCCESS;
}
