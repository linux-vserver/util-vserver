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

#include <lib/vserver.h>
#include <lib/internal.h>

#include <getopt.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#define ENSC_WRAPPERS_PREFIX	"keep-ctx-alive: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_SOCKET	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_IOSOCK	1
#define ENSC_WRAPPERS_IO	1
#include <wrappers.h>


#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_SOCKET		0x4000
#define CMD_TIMEOUT		0x4001
#define CMD_QUIT		0x4002

int 	wrapper_exit_code = 1;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,       0, CMD_HELP },
  { "version",    no_argument,       0, CMD_VERSION },
  { "socket",     required_argument, 0, CMD_SOCKET },
  { "timeout",    required_argument, 0, CMD_TIMEOUT },
  { "quit",       required_argument, 0, CMD_QUIT },
  {0,0,0,0},
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n    ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --socket <filename> [--timeout <seconds>]\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "keep-ctx-alive " VERSION " -- keeps a context alive\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static void
printXid(int fd)
{
  xid_t		xid = Evc_get_task_xid(0);
  char		buf[sizeof(xid)*3 + 2];
  size_t	l;

  l = utilvserver_fmt_long(buf, xid);
  EwriteAll(fd, buf, l);
  Eclose(fd);
}

static void handleMessage(int fd)
{
  char			buf[128];
  size_t		len;
  struct sockaddr_un	addr;
  socklen_t		addr_len = sizeof(addr);
  int			new_fd   = Eaccept(fd, &addr, &addr_len);

  len = Erecv(new_fd, buf, sizeof buf,0);
  if (len==0) exit(1);
  
    // TODO: handle message???
  exit(0);
}

static int
sendQuitSignal(char const *filename, char const *msg)
{
  int			fd;
  struct sockaddr_un	addr;

  ENSC_INIT_UNIX_SOCK(addr, filename);
  fd = Esocket(PF_UNIX, SOCK_STREAM, 0);
  Econnect(fd, &addr, sizeof(addr));

  if (msg) EsendAll(fd, msg, strlen(msg));
  Eclose(fd);

  return EXIT_SUCCESS;
}

static int
doit(char const *filename, struct timeval *timeout)
{
  int			fd;
  struct sockaddr_un	sock;
  pid_t			pid;
  fd_set		fd_set;

  ENSC_INIT_UNIX_SOCK(sock, filename);

  fd = Esocket(PF_UNIX, SOCK_STREAM, 0);
  Ebind(fd, &sock, sizeof sock);
  Elisten(fd, 5);

  printXid(1);

  FD_ZERO(&fd_set);
  FD_SET(fd, &fd_set);

  pid = Efork();
  if (pid==0) {
    Eselect(fd+1, &fd_set, 0,0, timeout);
    if (FD_ISSET(fd, &fd_set)) handleMessage(fd);
    exit(1);
  }

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  char const *		socket_name = 0;
  struct timeval	timeout = { 0,0 };
  struct timeval	*timeout_ptr = 0;
  char const *		quit_msg = false;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_SOCKET		:  socket_name = optarg; break;
      case CMD_QUIT		:  quit_msg    = optarg; break;
      case CMD_TIMEOUT		:
	timeout.tv_sec = atoi(optarg);
	timeout_ptr    = &timeout;
	break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return 255;
	break;
    }
  }

  if (socket_name==0)
    WRITE_MSG(2, "reserve-context: No socketname specified\n");
  if (quit_msg)
    return sendQuitSignal(socket_name, quit_msg);
  else
    return doit(socket_name, timeout_ptr);

  return EXIT_FAILURE;
}
