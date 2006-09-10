// $Id$    --*- c -*--

// Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#include "pathconfig.h"

#include <sys/stat.h>
#include <string.h>

static char const *
completePath(char const *id, size_t len, vcCfgStyle style, char *buf)
{
  switch (style) {
    case vcCFG_RECENT_FULL	:  return id;
    case vcCFG_RECENT_SHORT	:
      memcpy(buf,              CONFDIR "/", sizeof(CONFDIR "/")-1);
      memcpy(buf+sizeof(CONFDIR "/")-1, id, len+1);	// appends '\0' implicitly
      return buf;
    default			:  return 0;
  }
}

int
vc_compareVserverById(char const *lhs, vcCfgStyle lhs_style,
		      char const *rhs, vcCfgStyle rhs_style)
{
  if (lhs_style==vcCFG_NONE || lhs_style==vcCFG_AUTO)
    lhs_style = vc_getVserverCfgStyle(lhs);

  if (rhs_style==vcCFG_NONE || rhs_style==vcCFG_AUTO)
    rhs_style = vc_getVserverCfgStyle(rhs);

    // compare legacy vservers by their names only resp. return false on mixed
    // styles
  if (lhs_style==vcCFG_LEGACY || rhs_style==vcCFG_LEGACY) {
    if (lhs_style!=rhs_style) return lhs_style - rhs_style;
    else                      return strcmp(lhs, rhs);
  }

  {
    size_t		len_lhs = strlen(lhs);
    size_t		len_rhs = strlen(rhs);
    char		buf_lhs[sizeof(CONFDIR "//") + len_lhs];
    char		buf_rhs[sizeof(CONFDIR "//") + len_rhs];

    char const *	path_lhs = completePath(lhs, len_lhs, lhs_style, buf_lhs);
    char const *	path_rhs = (path_lhs==0
				    ? 0	// skip following calculation
				    : completePath(rhs, len_rhs, rhs_style, buf_rhs));

    struct stat		st_lhs;
    struct stat		st_rhs;
    
      // this is true only iff both path_* are 0; compare ids in this case
    if (path_lhs==path_rhs) return strcmp(lhs, rhs);
    if (path_lhs==0) return -1;		// path_rhs!=0 is implied by check above
    if (path_rhs==0) return +1;

    if (stat(path_lhs, &st_lhs)==-1 ||
	stat(path_rhs, &st_rhs)==-1) return strcmp(lhs,rhs);

    return (st_lhs.st_dev - st_rhs.st_dev) + (st_lhs.st_ino - st_rhs.st_ino);
  }
}
