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


#ifndef H_UTIL_VSERVER_LIB_SAFECHROOT_INTERNAL_H
#define H_UTIL_VSERVER_LIB_SAFECHROOT_INTERNAL_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include <stdlib.h>
#include <unistd.h>

#ifndef NDEBUG
static void
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


#endif	//  H_UTIL_VSERVER_LIB_SAFECHROOT_INTERNAL_H
