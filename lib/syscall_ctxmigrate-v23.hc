// $Id$    --*- c -*--

// Copyright (C) 2008 Daniel Hokka Zakrisson
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
#include <signal.h>
#include <lib_internal/sys_clone.h>

static inline ALWAYSINLINE int
vc_ctx_migrate_v23(xid_t xid, uint_least64_t flags)
{
  int ret;
  struct vcmd_ctx_migrate data = { .flagword = flags };
  int do_spaces = 0;

  ret = vc_getXIDType(xid);
  if (ret == vcTYPE_STATIC || ret == vcTYPE_DYNAMIC) {
    do_spaces = 1;
    ret = vc_enter_namespace(xid, vc_get_space_default());
    if (ret)
      return ret;
  }

  ret = vserver(VCMD_ctx_migrate, CTX_USER2KERNEL(xid), &data);

  if (!ret && do_spaces) {
    /* Allocate a new pid */
    int pid = sys_clone(SIGCHLD | CLONE_FS | CLONE_FILES, NULL);
    if (pid == -1)
      return -1;
    else if (pid > 0)
      vc_exitLikeProcess(pid, 1);
  }

  return ret;
}
