// $Id$    --*- c -*--

// Copyright (C) 2006 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#  error wrappers-termios.hc can not be used in this way
#endif

inline static WRAPPER_DECL void
Etcgetattr(int fd, struct termios *termios_p)		
{								
  FatalErrnoError(tcgetattr(fd, termios_p)==-1, "tcgetattr()");
}

inline static WRAPPER_DECL void
Etcsetattr(int fd, int optional_actions, struct termios *termios_p)		
{								
  FatalErrnoError(tcsetattr(fd, optional_actions, termios_p)==-1, "tcsetattr()");
}
