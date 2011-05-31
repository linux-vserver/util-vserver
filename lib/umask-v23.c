// $Id$    --*- c -*--

// Copyright (C) 2011 Asbjorn Sannes <asbjorn.sannes@interhost.no>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.h"
#include "internal.h"
#include <lib_internal/util-dimof.h>

#include <string.h>
#include <strings.h>
#include <assert.h>

#define DECL(STR, VAL) { STR, sizeof(STR)-1, VAL }

static struct Mapping_uint64 const VALUES[] = {
  DECL("fs",      CLONE_FS),
  DECL("newns",   CLONE_NEWNS),
  DECL("newuts",  CLONE_NEWUTS),
  DECL("newipc",  CLONE_NEWIPC),
  DECL("newuser", CLONE_NEWUSER),
  DECL("newpid",  CLONE_NEWPID),
  DECL("newnet",  CLONE_NEWNET),
};

uint_least64_t
vc_text2umask(char const *str, size_t len)
{
  ssize_t	idx = utilvserver_value2text_uint64(str, len,
						    VALUES, DIM_OF(VALUES));
  if (idx==-1) return 0;
  else         return VALUES[idx].val;
}

char const *
vc_loumask2text(uint_least64_t *val)
{
  ssize_t	idx = utilvserver_text2value_uint64(val,
						    VALUES, DIM_OF(VALUES));
  if (idx==-1) return 0;
  else         return VALUES[idx].id;
}
