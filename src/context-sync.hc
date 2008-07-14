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


/// \args p[0]  used for parent -> child sync (child waits till parent is in deathrow)
/// \args p[1]  used for child -> parent sync (recognize execv() errors)
static inline ALWAYSINLINE int
initSync(int p[2][2], bool do_disconnect)
{
  if (!do_disconnect) return 0;

  Epipe(p[0]);
  Epipe(p[1]);
  fcntl(p[1][1], F_SETFD, FD_CLOEXEC);
  return Efork();
}

static inline ALWAYSINLINE void
doSyncStage0(int p[2][2], bool do_disconnect)
{
  char		c;
  if (!do_disconnect) return;

  Eclose(p[0][1]);
  Eread (p[0][0], &c, 1);
  Eclose(p[0][0]);
}

static inline ALWAYSINLINE void
doSyncStage1(int p[2][2], bool do_disconnect)
{
  int	fd;

  if (!do_disconnect) return;
  
  fd = EopenD("/dev/null", O_RDONLY|O_NONBLOCK, 0);
  (void)setsid();	// ignore error when we are already a session-leader
  Edup2(fd, 0);
  Eclose(p[1][0]);
  if (fd!=0) Eclose(fd);
  Ewrite(p[1][1], ".", 1);
}

static inline ALWAYSINLINE void
doSyncStage2(int p[2][2], bool do_disconnect)
{
  if (!do_disconnect) return;

  Ewrite(p[1][1], "X", 1);
}

/// \args p[0]  used for parent -> child sync (child waits till parent is in deathrow)
/// \args p[1]  used for child -> parent sync (recognize execv() errors)
static void
waitOnSync(pid_t pid, int p[2][2], bool is_prevent_race)
{
  char		c;
  size_t	l;

  if (is_prevent_race &&
      !jailIntoTempDir(0)) {
    perror(ENSC_WRAPPERS_PREFIX "jailIntoTempDir()");
    exit(255);
  }

  Eclose(p[0][0]);
  Ewrite(p[0][1], "X", 1);
  Eclose(p[0][1]);
  
  Eclose(p[1][1]);
  l = Eread(p[1][0], &c, 1);
  if (l!=1) vc_exitLikeProcess(pid, wrapper_exit_code);
  l = Eread(p[1][0], &c, 1);
  if (l!=0) vc_exitLikeProcess(pid, wrapper_exit_code);
}
