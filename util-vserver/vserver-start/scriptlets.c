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

#include "vserver-start.h"

#include <pathconfig.h>
#include <lib_internal/command.h>
#include <lib_internal/util.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <dirent.h>

#define HAS_SUFFIX(STR, LEN, SUF) \
  (LEN>sizeof(SUF) && strcmp(STR+LEN-sizeof(SUF), SUF)==0)

static bool
visitFile(char const *fname, char const *vname, char const *style)
{
  struct stat		st;
  struct Command	cmd;

  if (stat(fname, &st)==-1 ||
      !S_ISREG(st.st_mode))
    return false;

  if ((st.st_mode & 0111)==0) {
    WRITE_MSG(2,
	      "!!!LEGACY ALERT!!!\n"
	      "The special handling of non-executable scriptlets which allows to\n"
	      "override environment variables is not supported anymore. This change\n"
	      "was needed as 'vserver ... start' is done by a native C program now.\n"
	      "If you need the old functionality please fill a bugreport so that\n"
	      "workarounds can be found/implemented.\n"
	      "The file triggering this message was\n"
	      "    '");
    WRITE_STR(2, fname);
    WRITE_MSG(2, "'\n");

    return false;
  }

  char const *par[] = { fname, style, vname, 0 };
  Command_setParams(&cmd, par);

  if (!Command_exec(&cmd, true) ||
      !Command_wait(&cmd, true)) {
    WRITE_MSG(2, "vserver-start: exec('");
    WRITE_STR(2, fname);
    WRITE_MSG(2, "'): ");
    WRITE_STR(2, strerror(cmd.err));
    WRITE_MSG(2, "; aborting...\n");

    exit(1);
  }

  if (cmd.rc!=0) {
    WRITE_MSG(2, "vserver-start: scriptlet '");
    WRITE_STR(2, fname);
    WRITE_MSG(2, "' failed; aborting...\n");
    
    exit (1);
  }

  Command_free(&cmd);

  return true;
}

static bool
visitDirentry(PathInfo const *basepath, char const *d_name,
	      char const *vname,
	      char const *style)
{
  size_t		l = strlen(d_name);
  char			path[basepath->l + l + 1];
  char *		ptr;

  if (isDotfile(d_name) ||
      HAS_SUFFIX(d_name, l, ".rpmnew") ||
      HAS_SUFFIX(d_name, l, ".rpmsave") ||
      HAS_SUFFIX(d_name, l, ".rpmorig") ||
      HAS_SUFFIX(d_name, l, ".cfsaved"))
    return false;

  ptr  = Xmemcpy(path, basepath->d, basepath->l);
  ptr  = Xmemcpy(ptr,  d_name,      l);
  *ptr = '\0';
  
  return visitFile(path, vname, style);
}

static bool
visitPath(PathInfo const *basepath,
	  char const *vname,
	  PathInfo const *style)
{
  char		tmp[basepath->l + style->l + sizeof(".d/")];
  PathInfo	path = { .d = tmp };
  char *	ptr;
  DIR *		dir;
  bool		did_something = false;

  ptr  = Xmemcpy(tmp, basepath->d, basepath->l);
  ptr  = Xmemcpy(ptr, style->d,    style->l);
  *ptr = '\0';
  path.l = ptr-tmp;

  did_something = visitFile(path.d, vname, style->d) || did_something;

  ptr = Xmemcpy(ptr, ".d/", sizeof(".d/"));
  path.l = ptr-tmp;
  
  dir = opendir(tmp);
  while (dir) {
    struct dirent *ent	=  readdir(dir);
    if (ent==0) break;

    did_something = visitDirentry(&path, ent->d_name, vname, style->d) || did_something;
  }
  if (dir!=0) closedir(dir);

  return did_something;
}

void
execScriptlets(PathInfo const *cfgdir, char const *vname, char const *style)
{
  char		path_buf[MAX(cfgdir->l, sizeof(CONFDIR "/.defaults")) +
			 sizeof("/scripts/")];
  PathInfo	basepath = { .d = path_buf };
  PathInfo	styledir = {
    .d = style,
    .l = strlen(style)
  };
  char *	ptr;
  bool		doit = true;

  ptr = Xmemcpy(path_buf, cfgdir->d, cfgdir->l);
  ptr = Xmemcpy(ptr,      "/scripts/", sizeof("/scripts/"));
  basepath.l = ptr-path_buf-1;
  doit =   !visitPath(&basepath, vname, &styledir);

  if (doit) {
    ptr = Xmemcpy(path_buf, CONFDIR "/.defaults/scripts/",
		  sizeof(CONFDIR "/.defaults/scripts/"));
    doit = !visitPath(&basepath, vname, &styledir);
  }
}
