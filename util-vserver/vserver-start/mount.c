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

#include "mount.h"
#include "configuration.h"
#include "undo.h"

#include <pathconfig.h>

#include <lib/internal.h>
#include <lib_internal/command.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>
#include <string.h>

#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#include <ensc_wrappers/wrappers.h>

static char const *
findMtab(char const *vserver_mtab)
{
  char const *tmp;
  
  if (utilvserver_isFile(vserver_mtab, true)) return vserver_mtab;

  tmp=CONFDIR "/.defaults/init/mtab";
  if (utilvserver_isFile(tmp, true)) return tmp;

  tmp=PKGLIBDEFAULTDIR "/mtab";
  if (utilvserver_isFile(tmp, true)) return tmp;

  return 0;
}

static void
initMtab(struct Configuration const *cfg)
{
  ENSC_PI_DECLARE(mtab_subpath,  "apps/init/mtab");
  PathInfo		mtab_path  = cfg->cfgdir;
  char			mtab_buf[ENSC_PI_APPSZ(mtab_path, mtab_subpath)];

  PathInfo_append(&mtab_path,  &mtab_subpath,  mtab_buf);
  char const *		mtab = findMtab(mtab_path.d);
  pid_t			pid;
  int			p[2];

  Epipe(p);
  pid = Efork();
  if (pid==0) {
    Undo_detach();
    Eclose(p[1]);
    
    Echdir(cfg->vdir);
    Echroot(".");

    int		fd = Eopen("/etc/mtab", O_WRONLY|O_CREAT, 0644);
    for (;;) {
      char	buf[4096];
      ssize_t	len = TEMP_FAILURE_RETRY(read(p[0], buf, sizeof buf));
      if (len==0) break;
      if (len==-1) {
	perror("vserver-start: initMtab/read():");
	_exit(1);
      }

      Ewrite(fd, buf, len);
    }
    Eclose(fd);
    Eclose(p[0]);
    _exit(0);
  }
  else {
    Eclose(p[0]);

    if (mtab!=0) {
      int		fd = Eopen(mtab, O_RDONLY, 0644);

      for (;;) {
	char	buf[4096];
	ssize_t	len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof buf));
	if (len==0) break;
      if (len==-1) {
	perror("vserver-start: initMtab/read():");
	_exit(1);
      }

	Ewrite(p[1], buf, len);
      }

      Eclose(fd);
    }

    Eclose(p[1]);

    int		status;
    TEMP_FAILURE_RETRY(wait4(pid, &status, 0,0));

    if (!WIFEXITED(status) || WEXITSTATUS(status)!=0) {
      exit(1);
    }
  }
}

static void
mountVserverInternal(struct Configuration const *cfg,
		     PathInfo const *path, bool use_chbind)
{
  if (!utilvserver_isFile(path->d,true)) return;

  pid_t		pid = Efork();
  if (pid==0) {
    Undo_detach();

    Echdir(cfg->vdir);

    if (use_chbind) {
	// TODO
    }

    struct Command	cmd;
    char const *	argv[] = {
      PROG_SECURE_MOUNT,
      "-a",
      "--chroot", ".",
      "--fstab", path->d,
      0
    };

    Command_init(&cmd);
    Command_setParams(&cmd, argv);
    Command_exec(&cmd, false);
  }
  else {
    int		status;
    TEMP_FAILURE_RETRY(wait4(pid, &status, 0,0));

    if (!WIFEXITED(status) || WEXITSTATUS(status)!=0)
      exit(1);
  }
}

void
mountVserver(struct Configuration const *cfg)
{
  ENSC_PI_DECLARE(fstab_subpath,  "fstab");
  ENSC_PI_DECLARE(fstabl_subpath, "fstab.local");

  PathInfo	fstab_path  = cfg->cfgdir;
  char		fstab_buf[ENSC_PI_APPSZ(fstab_path, fstab_subpath)];

  PathInfo	fstabl_path = cfg->cfgdir;
  char		fstabl_buf[ENSC_PI_APPSZ(fstabl_path, fstabl_subpath)];

  
  PathInfo_append(&fstab_path,  &fstab_subpath,  fstab_buf);
  PathInfo_append(&fstabl_path, &fstabl_subpath, fstabl_buf);
  initMtab(cfg);

  mountVserverInternal(cfg, &fstab_path,  true);
  mountVserverInternal(cfg, &fstabl_path, false);
}
