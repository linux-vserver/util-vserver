/*	--*- c -*--
 * Copyright (C) 2015 Enrico Scholz <enrico.scholz@ensc.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 and/or 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util.h"

#include <stdio.h>
#include <sys/mount.h>
#ifdef HAVE_LINUX_TYPES_H
#  include <linux/types.h>
#endif
#include <linux/fs.h>

#ifndef MS_REC
#define MS_REC		0x4000
#endif
#ifndef MS_PRIVATE
#define MS_PRIVATE	(1<<18)
#endif

bool cleanupMount(void)
{
  bool rc;

  /* systemd mounts everything with MS_SHARED which breaks our
   * filesystem mounting.  Revert mount status back to pre-systemd */
  rc = mount(NULL, "/", NULL, MS_REC|MS_PRIVATE, NULL) >= 0;
  if (!rc)
    perror("mount(\"/\", MS_REC|MS_PRIVATE)");

  return rc;
}
