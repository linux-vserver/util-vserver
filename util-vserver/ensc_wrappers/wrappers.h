// $Id$    --*- c++ -*--

// Copyright (C) 2003,2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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


#ifndef H_UTIL_VSERVER_SRC_WRAPPERS_H
#define H_UTIL_VSERVER_SRC_WRAPPERS_H

#define WRAPPER_DECL		UNUSED ALWAYSINLINE
#define H_ENSC_IN_WRAPPERS_H	1

#include "wrappers_handler.hc"

#ifdef ENSC_WRAPPERS_UNISTD
#  include "wrappers-unistd.hc"
#endif

#ifdef ENSC_WRAPPERS_FCNTL
#  include "wrappers-fcntl.hc"
#endif

#ifdef ENSC_WRAPPERS_MOUNT
#  include "wrappers-mount.hc"
#endif

#ifdef ENSC_WRAPPERS_RESOURCE
#  include "wrappers-resource.hc"
#endif

#ifdef ENSC_WRAPPERS_IOCTL
#  include "wrappers-ioctl.hc"
#endif

#ifdef ENSC_WRAPPERS_WAIT
#  include "wrappers-wait.hc"
#endif

#ifdef ENSC_WRAPPERS_VSERVER
#  include "wrappers-vserver.hc"
#endif

#ifdef ENSC_WRAPPERS_IO
#  include "wrappers-io.hc"
#endif

#ifdef ENSC_WRAPPERS_IOSOCK
#  include "wrappers-iosock.hc"
#endif

#ifdef ENSC_WRAPPERS_DIRENT
#  include "wrappers-dirent.hc"
#endif

#ifdef ENSC_WRAPPERS_CLONE
#  include "wrappers-clone.hc"
#endif

#ifdef ENSC_WRAPPERS_STDLIB
#  include "wrappers-stdlib.hc"
#endif

#undef H_ENSC_IN_WRAPPERS_H
#undef WRAPPER_DECL

#endif	//  H_UTIL_VSERVER_SRC_WRAPPERS_H
