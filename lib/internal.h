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

/** \file internal.h
 *  \brief Declarations which are used by util-vserver internally.
 */

#ifndef H_UTIL_VSERVER_LIB_INTERNAL_H
#define H_UTIL_VSERVER_LIB_INTERNAL_H

#include "fmt.h"
#include "vserver.h"

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

char *	vc_getVserverByCtx_Internal(xid_t ctx, /*@null@*/vcCfgStyle *style,
				    /*@null@*/char const *revdir,
				    bool validate_result);

  
int	utilvserver_checkCompatVersion();
uint_least32_t	utilvserver_checkCompatConfig();
bool	utilvserver_isDirectory(char const *path, bool follow_link);
bool	utilvserver_isFile(char const *path, bool follow_link);
bool	utilvserver_isLink(char const *path);

int	utilvserver_listparser_uint32(char const *str, size_t len,
				      char const **err_ptr, size_t *err_len,
				      uint_least32_t *flag,
				      uint_least32_t *mask,
				      uint_least32_t (*func)(char const*,
							     size_t, bool *
					)) NONNULL((1,5,7));
  
int	utilvserver_listparser_uint64(char const *str, size_t len,
				      char const **err_ptr, size_t *err_len,
				      uint_least64_t *flag,
				      uint_least64_t *mask,
				      uint_least64_t (*func)(char const*,
							     size_t, bool *
					)) NONNULL((1,5,7));

struct Mapping_uint32 {
    char const * const	id;
    size_t			len;
    uint_least32_t		val;
};

struct Mapping_uint64 {
    char const * const	id;
    size_t			len;
    uint_least64_t		val;
};
  
ssize_t	utilvserver_value2text_uint32(char const *str, size_t len,
				      struct Mapping_uint32 const *map,
				      size_t map_len) NONNULL((1,3));

ssize_t	utilvserver_value2text_uint64(char const *str, size_t len,
				      struct Mapping_uint64 const *map,
				      size_t map_len) NONNULL((1,3));

ssize_t utilvserver_text2value_uint32(uint_least32_t *val,
				      struct Mapping_uint32 const *map,
				      size_t map_len) NONNULL((1,2));
  
ssize_t utilvserver_text2value_uint64(uint_least64_t *val,
				      struct Mapping_uint64 const *map,
				      size_t map_len) NONNULL((1,2));
#ifdef __cplusplus
}
#endif


#endif	//  H_UTIL_VSERVER_LIB_INTERNAL_H
