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


#ifndef H_UTIL_VSERVER_VSERVER_START_INTERFACE_H
#define H_UTIL_VSERVER_VSERVER_START_INTERFACE_H

#include "configuration.h"

#include <lib_internal/util-cast.h>
#include <lib_internal/pathinfo.h>
#include <lib/vserver.h>
#include <stdbool.h>

struct Interface {
    union {
	struct {
	    uint32_t		ip;
	    uint32_t		mask;
	    uint32_t		extip;
	    uint32_t		bcast;
	}			ipv4;
    }				addr;
	
    char const *		name;
    char const *		scope;
    char const *		dev;
    char const *		mac;
    bool			nodev;
    bool			direct;
    bool			up;
};

void		activateInterfaces(InterfaceList const *interfaces);
void		deactivateInterfaces(InterfaceList const *interfaces);

static void	Iface_init(struct Interface *);
static void	Iface_free(struct Interface *);
bool		Iface_read(struct Interface *, PathInfo *cfgdir,
			   struct Interface const *dflt);
bool		Iface_add(struct Interface const *);
bool		Iface_del(struct Interface const *);
bool		Iface_remove(struct Interface const *);
void		Iface_print(struct Interface const *, int fd);

#include "interface-init.hc"
#include "interface-free.hc"

#endif	//  H_UTIL_VSERVER_VSERVER_START_INTERFACE_H
