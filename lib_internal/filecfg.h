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


#ifndef H_UTIL_VSERVER_LIB_INTERNAL_FILECFG_H
#define H_UTIL_VSERVER_LIB_INTERNAL_FILECFG_H

#include "pathinfo.h"
#include <stdbool.h>

// 1MiB should be enough for all applications
#define FILECFG_MAX_FILESIZE	0x100000

char *	FileCfg_readEntryStr (PathInfo const *base, char const *file, bool allow_multiline, char const *dflt);
bool	FileCfg_readEntryFlag(PathInfo const *base, char const *file, bool dflt);

typedef bool	(*FileCfg_MultiLineHandler)(void *res, char const *data, size_t len);

bool	FileCfg_iterateOverMultiLine(char const *str, FileCfg_MultiLineHandler handler,
				     void *data);

#endif	//  H_UTIL_VSERVER_LIB_INTERNAL_FILECFG_H
