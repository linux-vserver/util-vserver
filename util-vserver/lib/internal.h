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


#ifndef H_UTIL_VSERVER_LIB_INTERNAL_H
#define H_UTIL_VSERVER_LIB_INTERNAL_H

#include "fmt.h"
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int	utilvserver_checkCompatVersion();
bool	utilvserver_isDirectory(char const *path, bool follow_link);

int	utilvserver_listparser_uint32(char const *str, size_t len,
				      char const **err_ptr, size_t *err_len,
				      uint_least32_t *flag,
				      uint_least32_t *mask,
				      uint_least32_t (*func)(char const*,
							     size_t)) NONNULL((0,4,5,6));
  
int	utilvserver_listparser_uint64(char const *str, size_t len,
				      char const **err_ptr, size_t *err_len,
				      uint_least64_t *flag,
				      uint_least64_t *mask,
				      uint_least64_t (*func)(char const*,
							     size_t)) NONNULL((0,4,5,6));
  
#ifdef __cplusplus
}
#endif


#endif	//  H_UTIL_VSERVER_LIB_INTERNAL_H
