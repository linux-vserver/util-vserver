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


#ifndef H_ENSC_IN_WRAPPERS_H
#  error wrappers_handler.hc can not be used in this way
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static inline ALWAYSINLINE NORETURN void
FatalErrnoErrorFail(char const msg[])
{
  extern int	wrapper_exit_code;

#ifdef ENSC_WRAPPERS_PREFIX
  {
    int		old_errno = errno;
    WRITE_MSG(2, ENSC_WRAPPERS_PREFIX);
    errno = old_errno;
  }
#endif
  perror(msg);
  
  exit(wrapper_exit_code);
}

static UNUSED void 
FatalErrnoError(bool condition, char const msg[]) /*@*/
{
  if (!condition)       return;
  FatalErrnoErrorFail(msg);
}
