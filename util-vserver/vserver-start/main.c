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
#include "vshelper.h"
#include "pathconfig.h"
#include "interface.h"
#include "configuration.h"

#include "lib_internal/util.h"
#include "lib_internal/errinfo.h"
#include "lib/vserver.h"

#include <sys/file.h>

struct Options			opts;
struct Configuration		cfg;
int				wrapper_exit_code;

static void
env2Str(char const **var, char const *env, bool req)
{
  char const *		tmp = getenv(env);
  if (req && tmp==0) {
    WRITE_MSG(2, "vserver.start: required environment variable $");
    WRITE_STR(2, env);
    WRITE_STR(2, " not set; aborting...\n");
    exit(1);
  }

  *var = tmp;
  unsetenv(env);
}

static void
env2Bool(bool *var, char const *env, bool req)
{
  char const *	tmp;
  env2Str(&tmp, env, req);
  *var = !(tmp==0 || atoi(tmp)==0);
}

static void
initGlobals()
{
  env2Str (&opts.VSERVER_DIR,       "VSERVER_DIR",       true);
  env2Str (&opts.VSERVER_NAME,      "VSERVER_NAME",      true);
  env2Bool(&opts.OPTION_DEBUG,      "OPTION_DEBUG",      false);
  env2Bool(&opts.OPTION_DEFAULTTTY, "OPTION_DEFAULTTTY", false);  
}

static void
initLock()
{
  size_t			l   = strlen(opts.VSERVER_DIR);
  char 				tmp[sizeof(LOCKDIR "/vserver..startup") + l];
  char *			ptr = tmp;
  struct ErrorInformation	err = { .app = 0 };
  int				fd;

  ptr  = Xmemcpy(ptr, LOCKDIR "/vserver.", sizeof(LOCKDIR "/vserver.")-1);
  ((char *)(Xmemcpy(ptr, opts.VSERVER_DIR, l)))[0] = '\0';
  ptr += canonifyVserverName(ptr);
  ptr  = Xmemcpy(ptr, ".startup",  sizeof(".startup"));
  *ptr = '\0';

  if (!lockfile(&fd, tmp, LOCK_EX, 30, &err)) {
    WRITE_MSG(2, "vserver.start: failed to lock '");
    WRITE_STR(2, tmp);
    WRITE_MSG(2, "': ");
    ErrInfo_writeErrno(&err);
    exit(1);
  }
}

static void
checkConstraints()
{
  xid_t			xid;
  bool			is_running;
  struct vc_vx_info	info;

  xid = vc_getVserverCtx(opts.VSERVER_DIR, vcCFG_RECENT_FULL,
			 true, &is_running);

  if (xid!=VC_NOCTX && vc_get_vx_info(xid, &info)!=-1) {
    WRITE_MSG(2, "vserver.start: vserver '");
    WRITE_STR(2, opts.VSERVER_NAME);
    WRITE_MSG(2, "' already running; aborting...\n");
    exit(1);
  }

  Vshelper_doSanityCheck();
}

int main(int argc, char *argv[])
{
  Cfg_init(&cfg);
  
  initGlobals();
  initLock();
  checkConstraints();

  PathInfo	cfgdir = { .d = opts.VSERVER_DIR, .l = strlen(opts.VSERVER_DIR) };

  getConfiguration(&cfg, &cfgdir);
  execScriptlets(&cfgdir, opts.VSERVER_NAME, "prepre-start");
  activateInterfaces();
}
