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


static uint64_t
visitDirEntry(struct dirent const *ent);

static uint64_t
visitDir(char const *name, struct stat const *expected_stat)
{
  int		fd = Eopen(".", O_RDONLY, 0);
  PathInfo	old_state = global_info.state;
  PathInfo	rhs_path = {
    .d = name,
    .l = strlen(name)
  };
  char		new_path[ENSC_PI_APPSZ(global_info.state, rhs_path)];
  DIR *		dir;
  uint64_t	res = 0;

  PathInfo_append(&global_info.state, &rhs_path, new_path);

  if (expected_stat!=0)
    EsafeChdir(name, expected_stat);
  
  dir = Eopendir(".");

  for (;;) {
    struct dirent		*ent = Ereaddir(dir);
    if (ent==0) break;

    res += visitDirEntry(ent);
  }

  Eclosedir(dir);

  Efchdir(fd);
  Eclose(fd);

  global_info.state = old_state;
  return res;
}
