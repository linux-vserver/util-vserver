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


#ifndef H_UTIL_VSERVER_VSERVER_START_CONFIGURATION_H
#define H_UTIL_VSERVER_VSERVER_START_CONFIGURATION_H

#include <lib/vserver.h>
#include <ensc_vector/vector.h>
#include <lib_internal/pathinfo.h>

#include <stdbool.h>

typedef struct Vector	InterfaceList;

struct Configuration {
    PathInfo		cfgdir;
    char const *	vdir;
    xid_t		xid;
    uint32_t		broadcast;
    InterfaceList	interfaces;
    char const *	nice;

    struct vc_ctx_caps	ctx_caps;
    struct vc_ctx_flags	ctx_flags;
};

  //extern struct Configuration		cfg;

void		Cfg_init(struct Configuration *);
bool		getConfiguration(struct Configuration *, PathInfo const *cfgdir);


#endif	//  H_UTIL_VSERVER_VSERVER_START_CONFIGURATION_H
