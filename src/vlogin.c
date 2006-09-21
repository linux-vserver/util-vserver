// $Id$

// Copyright (C) 2006 Benedikt Böhm <hollow@gentoo.org>
// Based on vserver-utils' vlogin program.
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
#include <lib/fmt.h>

#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <termios.h>
#include <signal.h>
#include <pty.h>
#include <fcntl.h>

#define ENSC_WRAPPERS_PREFIX	"vlogin: "
#define ENSC_WRAPPERS_IOCTL	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_SOCKET	1
#define ENSC_WRAPPERS_IO	1
#define ENSC_WRAPPERS_TERMIOS	1
#define ENSC_WRAPPERS_FCNTL	1
#include <wrappers.h>

struct terminal {
  int fd;                           /* terminal file descriptor */
  struct termios term;              /* terminal settings */
  struct winsize ws;                /* terminal size */
  pid_t pid;                        /* terminal process id */
  struct termios termo;             /* original terminal settings */
  enum { TS_RESET, TS_RAW } state;  /* terminal state */
};

static struct terminal t;
extern int wrapper_exit_code;

/* set terminal to raw mode */
static void
terminal_raw(void)
{
  struct termios buf;

  /* save original terminal settings */
  Etcgetattr(STDIN_FILENO, &t.termo);

  buf = t.termo;
  
  /* convert terminal settings to raw mode */
  cfmakeraw(&buf);

  /* apply raw terminal settings */
  Etcsetattr(STDIN_FILENO, TCSAFLUSH, &buf);

  t.state = TS_RAW;
}

/* reset terminal to original state */
static void
terminal_reset(void)
{
  if (t.state != TS_RAW)
    return;

  Etcsetattr(STDIN_FILENO, TCSAFLUSH, &t.termo);

  t.state = TS_RESET;
}

/* send signal to terminal */
static void
terminal_kill(int sig)
{
  pid_t pgrp = -1;

  /* try to get process group leader */
  if (ioctl(t.fd, TIOCGPGRP, &pgrp) >= 0 &&
      pgrp != -1 &&
      kill(-pgrp, sig) != -1)
    return;

  /* fallback using terminal pid */
  kill(-t.pid, sig);
}

/* redraw the terminal screen */
static void
terminal_redraw(void)
{
  /* get winsize from stdin */
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &t.ws) == -1)
    return;

  /* set winsize in terminal */
  ioctl(t.fd, TIOCSWINSZ, &t.ws);

  /* set winsize change signal to terminal */
  terminal_kill(SIGWINCH);
}

/* copy terminal activities */
static void
terminal_copy(int src, int dst)
{
  char buf[64];
  ssize_t len;

  /* read terminal activity */
  len = read(src, buf, sizeof(buf));
  if (len == -1 && errno != EINTR) {
    perror("read()");
    terminal_kill(SIGTERM);
    exit(1);
  } else if (len == -1)
    return;

  /* write activity to user */
  EwriteAll(dst, buf, len);
}

/* shuffle all output, and reset the terminal */
static void
terminal_end(void)
{
  char buf[64];
  ssize_t len;
  long options;

  options = Efcntl(t.fd, F_GETFL, 0) | O_NONBLOCK;
  Efcntl(t.fd, F_SETFL, options);
  for (;;) {
    len = read(t.fd, buf, sizeof(buf));
    if (len == 0 || len == -1)
      break;
    EwriteAll(STDOUT_FILENO, buf, len);
  }

  /* in case atexit hasn't been setup yet */
  terminal_reset();
}

/* catch signals */
static void
signal_handler(int sig)
{
  int status;

  switch(sig) {
    /* catch interrupt */
    case SIGINT:
      terminal_kill(sig);
      break;

    /* terminal died */
    case SIGCHLD:
      terminal_end();
      wait(&status);
      exit(WEXITSTATUS(status));
      break;

    /* window size has changed */
    case SIGWINCH:
      terminal_redraw();
      break;

    default:
      exit(0);
  }

}

void do_vlogin(int argc, char *argv[], int ind)
{
  int slave;
  pid_t pid;
  int n, i;
  fd_set rfds;

  if (!isatty(0) || !isatty(1)) {
    execvp(argv[ind], argv+ind);
    return;
  }

  /* set terminal to raw mode */
  terminal_raw();

  /* reset terminal to its original mode */
  atexit(terminal_reset);

  /* fork new pseudo terminal */
  if (openpty(&t.fd, &slave, NULL, NULL, NULL) == -1) {
    perror(ENSC_WRAPPERS_PREFIX "openpty()");
    exit(EXIT_FAILURE);
  }

  /* setup SIGCHLD here, so we're sure to get the signal */
  signal(SIGCHLD, signal_handler);

  pid = Efork();

  if (pid == 0) {
    /* we don't need the master side of the terminal */
    close(t.fd);

    /* login_tty() stupid dietlibc doesn't have it */
    Esetsid();

    Eioctl(slave, TIOCSCTTY, NULL);

    Edup2(slave, 0);
    Edup2(slave, 1);
    Edup2(slave, 2);

    if (slave > 2)
      close(slave);

    Eexecvp(argv[ind], argv+ind);
  }

  /* setup SIGINT and SIGWINCH here, as they can cause loops in the child */
  signal(SIGWINCH, signal_handler);
  signal(SIGINT, signal_handler);

  /* save terminals pid */
  t.pid = pid;

  /* set process title for ps */
  n = strlen(argv[0]);

  for (i = 0; i < argc; i++)
    memset(argv[i], '\0', strlen(argv[i]));

  strncpy(argv[0], "login", n);

  /* we want a redraw */
  terminal_redraw();

  /* main loop */
  for (;;) {
    /* init file descriptors for select */
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    FD_SET(t.fd, &rfds);
    n = t.fd;

    /* wait for something to happen */
    while (select(n + 1, &rfds, NULL, NULL, NULL) == -1) {
      if (errno == EINTR || errno == EAGAIN)
	continue;
      perror(ENSC_WRAPPERS_PREFIX "select()");
      exit(wrapper_exit_code);
    }

    if (FD_ISSET(STDIN_FILENO, &rfds))
      terminal_copy(STDIN_FILENO, t.fd);

    if (FD_ISSET(t.fd, &rfds))
      terminal_copy(t.fd, STDOUT_FILENO);
  }

  /* never get here, signal handler exits */
}
