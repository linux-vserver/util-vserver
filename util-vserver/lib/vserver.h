// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef H_VSERVER_SYSCALL_H
#define H_VSERVER_SYSCALL_H

#include <sys/types.h>
#include <unistd.h>

#define CTX_NOCTX	((ctx_t)(-1))

#ifdef __cplusplus
extern "C" {
#endif

  typedef short int	ctx_t;

int call_new_s_context(int nbctx, int ctxs[], int remove_cap, int flags);
int call_set_ipv4root (unsigned long ip[], int nb,
		       unsigned long bcast, unsigned long mask[]);
int call_chrootsafe (const char *dir);
int has_chrootsafe();
int call_set_ctxlimit (int res, long limit);

void	vserver_init();

  ctx_t		getctx(pid_t pid);

#define getcctx()	(getctx(getpid()))

#ifdef __cplusplus
}
#endif

#endif
