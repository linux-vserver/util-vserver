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


#ifndef H_UTIL_VSERVER_LIB_INTERNAL_COMMAND_H
#define H_UTIL_VSERVER_LIB_INTERNAL_COMMAND_H

#include <ensc_vector/vector.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdbool.h>

struct Command
{
    char const *	filename;
    struct Vector	params;
    pid_t		pid;
    int			rc;
    int			err;
    struct rusage	rusage;
};

void	Command_init(struct Command *, size_t param_count);
void	Command_free(struct Command *);
void	Command_reset(struct Command *);
bool	Command_exec(struct Command *, bool do_fork);
void	Command_appendParameter(struct Command *, char const *);
/**
 *  \args do_hang  when true, do not return before command exited, or
 *                 an error (e.g. signal) occured
 *  \returns       \c true iff command/processes exited; in this case,
 *                 exitcode is available in the \c rc member
 */
bool	Command_wait(struct Command *, bool do_block);

#endif	//  H_UTIL_VSERVER_LIB_INTERNAL_COMMAND_H
