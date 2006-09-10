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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "undo.h"

#include <lib_internal/util.h>
#include <ensc_vector/vector.h>

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

struct FuncData
{
    ExitFunction 		fn;
    void const *		args;
};

struct Undo
{
    pid_t		pid_;
    struct Vector	funcs;
};


static struct Undo	undo_data = {
  .pid_ = -1
};

static void
atexitHandler()
{
  struct FuncData const *	ptr;

  if (undo_data.pid_ != getpid())
    return;	// skip 'exit()' from forked processes

  for (ptr=Vector_end(&undo_data.funcs);
       ptr!=Vector_begin(&undo_data.funcs);
       --ptr)
    (ptr[-1].fn)(ptr[-1].args);
}

void
Undo_init()
{
  if (undo_data.pid_!=-1) {
    WRITE_MSG(2, "Undo already initialized; internal error...\n");
    _exit(1);
  }

  undo_data.pid_ = getpid();
  Vector_init(&undo_data.funcs, sizeof(struct FuncData));
  
  atexit(&atexitHandler);
}

void
Undo_addTask(ExitFunction fn, void const *args)
{
  struct FuncData	*tmp = Vector_pushback(&undo_data.funcs);
  assert(tmp!=0); // Vector_pushback never returns a null-pointer

  tmp->fn   = fn;
  tmp->args = args;
}

void
Undo_detach()
{
  Vector_free(&undo_data.funcs);
  undo_data.pid_ = -1;
}
