// $Id$    --*- c -*--

// Copyright (C) 2004,2015 Enrico Scholz <enrico.scholz@ensc.de>
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


#ifndef H_UTILVSERVER_LIB_INTERNAL_UTIL_H
#define H_UTILVSERVER_LIB_INTERNAL_UTIL_H

#include "util-cast.h"
#include "util-commonstrings.h"
#include "util-debug.h"
#include "util-declarecmd.h"
#include "util-dimof.h"
#include "util-dotfile.h"
#include "util-io.h"
#include "util-lockfile.h"
#include "util-mem.h"
#include "util-perror.h"
#include "util-safechdir.h"
#include "util-unixsock.h"

bool		switchToWatchXid(char const **);
size_t		canonifyVserverName(char *);
bool		isNumber(char const *, signed long *result, bool is_strict);
bool		isNumberUnsigned(char const *, unsigned long *result, bool is_strict);
bool		mkdirRecursive(char const *);

#endif	//  H_UTILVSERVER_LIB_INTERNAL_UTIL_H
