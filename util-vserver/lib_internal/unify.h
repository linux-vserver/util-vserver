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


#ifndef H_UTIL_VSERVER_LIB_INTERNAL_UNIFY_H
#define H_UTIL_VSERVER_LIB_INTERNAL_UNIFY_H

#include <stdbool.h>

struct stat;

bool
Unify_unify(char const *src, struct stat const *src_stat,
	    char const *dst, struct stat const *dst_stat) NONNULL((1,2,3,4));

bool
Unify_deUnify(char const *src, struct stat const *src_stat,
	      char const *dst, struct stat const *dst_stat) NONNULL((1,2,3,4));


#define	Unify_isUnified(LHS, RHS)		\
  ((bool)((LHS)->st_dev ==(RHS)->st_dev  &&	\
	  (LHS)->st_ino ==(RHS)->st_ino))

#define Unify_isUnifyable(LHS, RHS)		\
  ((bool)((LHS)->st_dev  ==(RHS)->st_dev  &&	\
	  (LHS)->st_ino  !=(RHS)->st_ino  &&	\
	  (LHS)->st_mode ==(RHS)->st_mode &&	\
	  (LHS)->st_uid  ==(RHS)->st_uid  &&	\
	  (LHS)->st_gid  ==(RHS)->st_gid  &&	\
	  (LHS)->st_size ==(RHS)->st_size &&	\
	  (LHS)->st_mtime==(RHS)->st_mtime))
  

#endif	//  H_UTIL_VSERVER_LIB_INTERNAL_UNIFY_H
