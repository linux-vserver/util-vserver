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

#include "pathconfig.h"
#include "lib_internal/util-dotfile.h"

#include <sys/param.h>

static size_t
initHashList(HashDirCollection *hash_vec, char const *hashdir)
{
  int		cur_dir = Eopen(".", O_RDONLY|O_DIRECTORY, 0);
  Echdir(hashdir);
  
  DIR		*d      = Eopendir(".");
  struct dirent	*ep;
  size_t	l   = strlen(hashdir);
  size_t	res = 0;

  while ((ep=readdir(d)) != 0) {
    struct stat		st;

    if (isDotfile(ep->d_name) ||
	stat(ep->d_name, &st)==-1 || !S_ISDIR(st.st_mode))
      continue;

    if (HashDirInfo_findDevice(hash_vec, st.st_dev)!=0) {
      WRITE_MSG(2, "Duplicate hash-dir entry '");
      WRITE_STR(2, ep->d_name);
      WRITE_MSG(2, "' found\n");
      continue;
    }

    char		*full_path = Emalloc(l + strlen(ep->d_name) + 3);
    char		*ptr       = full_path + l;

    memcpy(full_path, hashdir, l);
    while (ptr>full_path && ptr[-1]=='/') --ptr;
    *ptr++ = '/';
    strcpy(ptr, ep->d_name);
    strcat(ptr, "/");	// append a trailing '/'


    struct HashDirInfo	tmp = {
      .device  = st.st_dev,
      .path    = { full_path, strlen(full_path) },
    };

    res = MAX(res, tmp.path.l);
    
    memcpy(Vector_pushback(hash_vec), &tmp, sizeof tmp);
  }

  if (Vector_count(hash_vec)==0) {
    WRITE_MSG(2, "Could not find a place for the hashified files at '");
    WRITE_STR(2, hashdir);
    WRITE_MSG(2, "'.\n");
    exit(wrapper_exit_code);
  }

  Eclosedir(d);
  Efchdir(cur_dir);
  Eclose(cur_dir);

  return res;
}

static char *
searchHashdir(char const *lhs, char const *rhs)
{
  size_t	l1  = strlen(lhs);
  size_t	l2  = rhs ? strlen(rhs) : 0;
  char *	res = Emalloc(l1 + l2);
  struct stat	st;

  strcpy(res, lhs);
  if (rhs) strcat(res, rhs);

  if (stat(res, &st)==-1 || !S_ISDIR(st.st_mode)) {
    free(res);
    res = 0;
  }

  return res;
}

static void
initModeManually(struct Arguments const UNUSED *args, int argc, char *argv[])
{
  assert(args->hash_dir!=0);
  
  if (argc<2) {
    WRITE_MSG(2, "No exclude list specified\n");
    exit(1);
  }

  global_info.hash_dirs_max_size = initHashList(&global_info.hash_dirs,
						args->hash_dir);
  MatchList_initManually(&global_info.dst_list, 0, strdup(argv[0]), argv[1]);
}

static void
initModeVserver(struct Arguments const UNUSED *args, int argc, char *argv[])
{
  char const				*appdir;
  char const				*hashdir   = args->hash_dir;
  struct MatchVserverInfo const		dst_vserver = { argv[0], true };

  if (argc!=1) {
    WRITE_MSG(2, "More than one vserver is not supported\n");
    exit(1);
  }

  if (!MatchList_initByVserver(&global_info.dst_list, &dst_vserver, &appdir)) {
    WRITE_MSG(2, "unification not configured for this vserver\n");
    exit(1);
  }

  if (hashdir==0) hashdir = searchHashdir(appdir, "/hash");
  if (hashdir==0) hashdir = searchHashdir(CONFDIR "/.defaults/apps/vunify/hash", 0);

  if (hashdir==0) {
    WRITE_MSG(2, "no hash-directory configured for this vserver.\n");
    exit(1);
  }

  global_info.hash_dirs_max_size = initHashList(&global_info.hash_dirs, hashdir);

  free(const_cast(char *)(hashdir));
  free(const_cast(char *)(appdir));
}
		
