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

#include "matchlist.h"
#include "util-io.h"

#include <dirent.h>
#include <string.h>
#include <fcntl.h>

#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_STDLIB	1
#include <wrappers.h>

static int
selectRefserver(struct dirent const *ent)
{
  return strncmp(ent->d_name, "refserver.", 10)==0;
}

void
MatchList_initRefserverList(struct MatchList **lst, size_t *cnt,
			    char const *dir)
{
  int			cur_dir = Eopen(".", O_RDONLY, 0);
  struct dirent		**entries;
  int			count,i;
  
  Echdir(dir);
  count = scandir(".", &entries, selectRefserver, alphasort);
  if (count==-1) {
    perror("scandir()");
    exit(1);
  }

  if (count==0) {
    WRITE_MSG(2, "no reference vserver configured\n");
    exit(1);
  }

  *lst = Emalloc(sizeof(struct MatchList) * count);
  *cnt = count;
  for (i=0; i<count; ++i) {
    char const 		*tmp   = entries[i]->d_name;
    size_t		l      = strlen(tmp);
    char		vname[sizeof("./") + l];

    memcpy(vname,   "./", 2);
    memcpy(vname+2, tmp,  l+1);
    
    if (!MatchList_initByVserver((*lst)+i, vname, 0)) {
      WRITE_MSG(2, "unification for reference vserver not configured\n");
      exit(1);
    }

    free(entries[i]);
  }
  free(entries);

  Efchdir(cur_dir);
  Eclose(cur_dir);
}
