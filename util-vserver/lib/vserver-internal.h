// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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


#ifndef H_VSERVER_SYSCALL_INTERNAL_H
#define H_VSERVER_SYSCALL_INTERNAL_H

#include <stdint.h>
#include <syscall.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <errno.h>

#ifndef __NR_sys_virtual_context
#  define __NR_sys_virtual_context	273
#endif

#ifndef NDEBUG
static ALWAYSINLINE UNUSED void
vc_tell_unsafe_chroot()
{
  static int			flag = -1;
  if (flag==-1) {
    char const * const	e = getenv("VC_TELL_UNSAFE_CHROOT");
    flag = e ? atoi(e) : 0;
    flag = flag ? 1 : 0;
  }

  if (flag) write(2, "Unsafe chroot() used\n", 23);
}
#else
static ALWAYSINLINE UNUSED void	vc_tell_unsafe_chroot() {}
#endif

#ifndef HAVE_SYS_VIRTUAL_CONTEXT
static _syscall3(int, sys_virtual_context,
		 uint32_t, cmd, uint32_t, id, void *, data)
#endif
  
#endif	//  H_VSERVER_SYSCALL_INTERNAL_H
