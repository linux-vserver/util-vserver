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

#include "filecfg.h"

bool
FileCfg_iterateOverMultiLine(char const *str,
			     FileCfg_MultiLineHandler handler,
			     void *data)
{
  char const	*ptr    = str;
  size_t	line_nr = 1;
  
  while (*ptr!='\0') {
    while (*ptr==' ' || *ptr=='\t') ++ptr;	// left-trim line
    
    char const	*eol = strchr(ptr, '\n');
    if (eol==0) eol=ptr+strlen(ptr);	// handle unterminated lines

    if (*ptr!='#') {	// skip commented lines
      size_t	len = eol-ptr;
      while (len>0 && (ptr[len-1]==' ' || ptr[len-1]=='\t'))
	--len;		// right-trim line

	// handle only non-empty lines
      if (len>0 && !(*handler)(data, ptr, len))
	return false;	// some parsing-error occured...
    }
    
    ++line_nr;
    ptr = eol;
    if (*ptr) ++ptr;
  }

  return true;
}
