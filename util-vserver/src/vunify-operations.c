// $Id$    --*- c -*--

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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vunify-operations.h"
#include "util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>


// ======   The compare functions   =========

// return 'true' iff both files are different and unifyable 
static bool
compareUnify(struct stat const *lhs, struct stat const *rhs)
{
  return (lhs->st_dev ==rhs->st_dev  &&
	  lhs->st_ino !=rhs->st_ino  &&
	  lhs->st_mode==rhs->st_mode &&
	  lhs->st_uid ==rhs->st_uid  &&
	  lhs->st_gid ==rhs->st_gid  &&
	  lhs->st_size==rhs->st_size);

  // TODO: acl? time?
}

// return 'true' iff both files are the same one
static bool
compareDeUnify(struct stat const *lhs, struct stat const *rhs)
{
  return (lhs->st_dev ==rhs->st_dev  &&
	  lhs->st_ino ==rhs->st_ino);
}

// =====  The 'doit' functions   ===========


static bool
unifyTest(char const *src, char const *dst)
{
#if 0
  WRITE_MSG(1, "unifying '");
  WRITE_STR(1, src);
  WRITE_MSG(1, "' -> '");
  WRITE_STR(1, dst);
  WRITE_STR(1, "'\n");
#endif
  
  return true;
}


static struct Operations	op_unify_test = {
  .compare = compareUnify,
  .doit    = unifyTest
};

static struct Operations	op_deunify_test = {
  .compare = compareDeUnify,
  .doit    = unifyTest
};
  
static struct Operations	op_unify_notest = {
  .compare = compareUnify,
  .doit    = unifyTest
};

static struct Operations	op_deunify_notest = {
  .compare = compareDeUnify,
  .doit    = unifyTest
};

// ======

void
Operations_init(struct Operations *op, OperationType type, bool is_test)
{
  switch (type) {
    case opUNIFY	:
      if (is_test) *op = op_unify_test;
      else         *op = op_unify_notest;
      break;

    case opDEUNIFY	:
      if (is_test) *op = op_deunify_test;
      else         *op = op_deunify_notest;
      break;

    default		:
      abort();
  }
}
