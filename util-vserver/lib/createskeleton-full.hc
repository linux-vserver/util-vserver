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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define CONCAT_TWO_ARGS(BUF, LHS,RHS)			\
  size_t		BUF ## l1 = strlen(LHS);	\
  size_t		BUF ## l2 = strlen(RHS);	\
  char			BUF[BUF ## l1 + BUF ## l2 + 2];	\
  							\
  memcpy(BUF,      LHS, BUF ## l1 + 1);			\
  if (BUF ## l2>0) {					\
    memcpy(BUF+BUF ## l1,   "/", BUF ## l1);		\
    memcpy(BUF+BUF ## l1+1, RHS, BUF ## l2+1);		\
  }

static inline int
mkdir2(char const *lhs, char const *rhs, int mode)
{
  CONCAT_TWO_ARGS(buf, lhs, rhs);
  return mkdir(buf, mode);
}

static inline int
setIAttr2(char const *lhs, char const *rhs, int flags)
{
  CONCAT_TWO_ARGS(buf, lhs, rhs);

  return vc_set_iattr(buf, 0, flags, VC_IMMUTABLE_ALL);
}

static inline int
symlink2(char const *old_lhs, char const *old_rhs,
	 char const *new_lhs, char const *new_rhs)
{
  CONCAT_TWO_ARGS(old_buf, old_lhs, old_rhs);

  {
    CONCAT_TWO_ARGS(new_buf, new_lhs, new_rhs);
    return symlink(old_buf, new_buf);
  }
}

#undef CONCAT_TWO_ARGS

static inline int
vc_createSkeleton_full(char const *id, char const *name, int flags)
{
  if (mkdir(id, 0755)==-1) return -1;

  if (mkdir2(id, "apps", 0755)==-1 ||
      ((flags&vcSKEL_INTERFACES) && mkdir2(id, "interfaces", 755)==-1) ||
      ((flags&vcSKEL_PKGMGMT) && (
	mkdir2(id, "apps/pkgmgmt", 0755)==-1)))
    return -1;

  for (;;) {
    char const	*basedir = CONFDIR "/.defaults/run";

    if (!utilvserver_isDirectory(basedir, true)) basedir = DEFAULT_PKGSTATEDIR;
    if (!utilvserver_isDirectory(basedir, true)) break;

    if (symlink2(basedir, name, id, "run")==-1)
      return -1;

    break;
  }
  
  while (flags&vcSKEL_PKGMGMT) {
    char const	*basedir = CONFDIR "/.defaults/apps/pkgmgmt/base";

    if (!utilvserver_isDirectory(basedir, true)) basedir = DEFAULT_VSERVERPKGDIR;
    if (!utilvserver_isDirectory(basedir, true)) break;

    if (mkdir2(basedir, name, 0755)==-1 ||
	symlink2(basedir, name, id, "apps/pkgmgmt/base")==-1)
      return -1;

    break;
  }

  while (flags&vcSKEL_FILESYSTEM) {
    char const	*basedir = CONFDIR "/.defaults/vdirbase";

    if (!utilvserver_isDirectory(basedir, true)) basedir = DEFAULT_VSERVERDIR;
    if (!utilvserver_isDirectory(basedir, true)) break;

    if (mkdir2(basedir, name, 0755)==-1 ||
	setIAttr2(basedir, name, 0)==-1 ||
	symlink2(basedir, name, id, "vdir")==-1)
      return -1;

    break;
  }

  return 0;
}
