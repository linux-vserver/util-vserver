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

#include "fstool.h"
#include "util.h"

#include <lib/vserver.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#define ENSC_WRAPPERS_DIRENT	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

struct Arguments const *		global_args = 0;

int wrapper_exit_code = 1;

bool
checkForRace(int fd, char const * name, struct stat const *exp_st)
{
  struct stat		st;
  
  if (fstat(fd, &st)==-1) {
    perror("fstat()");
    return false;
 }

  if (st.st_dev  != exp_st->st_dev ||
      st.st_ino  != exp_st->st_ino ||
      st.st_mode != exp_st->st_mode) {
    close(fd);
    WRITE_MSG(2, "race while visiting '");
    WRITE_STR(2, name);
    WRITE_MSG(2, "'\n");
    exit(2);
  }

  return true;
}

inline static bool
isSpecialDir(char const *d)
{
  return ( (d[0]=='.' && !global_args->do_display_dot) ||
	   (d[0]=='.' && (d[1]=='\0' || (d[1]=='.' && d[2]=='\0'))) );
}

#define CONCAT_PATHS(LHS, LHS_LEN, RHS)					\
  size_t		l_rhs = strlen(RHS);				\
  char			new_path[(LHS_LEN) + l_rhs + sizeof("/")];	\
  memcpy(new_path, LHS, (LHS_LEN));					\
  memcpy(new_path+(LHS_LEN), "/", 1);					\
  memcpy(new_path+(LHS_LEN)+1, RHS, l_rhs);				\
  new_path[(LHS_LEN)+1+l_rhs] = '\0';

static uint64_t
iterateFilesystem(char const *path)
{
  bool			do_again = false;
  size_t		path_len = strlen(path);
  uint64_t		err = 0;
  struct stat		cur_st;
  DIR *			dir = opendir(".");

  if (dir==0) {
    perror("opendir()");
    return 1;
  }

  // show current directory entry first
  if (lstat(".", &cur_st)==-1) perror("lstat()");
  else err += handleFile(".", path) ? 0 : 1;

  // strip trailing '/'
  while (path_len>0 && path[path_len-1]=='/') --path_len;

  // process regular files before directories
  for (;;) {
    struct dirent	*ent = Ereaddir(dir);
    struct stat		st;
    
    if (ent==0) break;
    if (isSpecialDir(ent->d_name)) continue;

    if (lstat(ent->d_name, &st)==-1) {
      perror("lstat()");
      ++err;
      continue;
    }

    if (S_ISDIR(st.st_mode) && global_args->do_recurse) {
      do_again = true;
      continue;
    }
    
    {
      CONCAT_PATHS(path, path_len, ent->d_name);
      err += handleFile(ent->d_name, new_path) ? 0 : 1;
    }
  }

  if (do_again) {
    int		cur_dir = Eopen(".", O_RDONLY, 0);
    rewinddir(dir);

    for (;;) {
      struct dirent	*ent = Ereaddir(dir);
      struct stat	st;
    
      if (ent==0) break;
      if (isSpecialDir(ent->d_name)) continue;
      
      if (lstat(ent->d_name, &st)==-1) {
	perror("lstat()");
	++err;
	continue;
      }

      if (!S_ISDIR(st.st_mode) ||
	  (global_args->local_fs && st.st_dev!=cur_st.st_dev))
	continue;

      if (safeChdir(ent->d_name, &st)==-1) {
	perror("chdir()");
	++err;
	continue;
      }
      
      {
	CONCAT_PATHS(path, path_len, ent->d_name);
	err += iterateFilesystem(new_path);
      }
      Efchdir(cur_dir);
    }
    Eclose(cur_dir);
  }

  Eclosedir(dir);

  return err;
}
#undef CONCAT_PATHS

static uint64_t
processFile(char const *path)
{
  struct stat		st;

  if (lstat(path, &st)==-1) {
    perror("lstat()");
    return 1;
  }

  if (S_ISDIR(st.st_mode) && !global_args->do_display_dir) {
    Echdir(path);
    return iterateFilesystem(path);
  }
  else
    return handleFile(path, path) ? 0 : 1;
}

int main(int argc, char *argv[])
{
  uint64_t		err_cnt = 0;
  int			i;
  struct Arguments	args = {
    .do_recurse		=  false,
    .do_display_dot	=  false,
    .do_display_dir 	=  false,
    .do_mapping		=  true,
    .ctx		=  VC_NOCTX,
    .is_legacy          =  false,
    .do_set             =  false,
    .do_unset           =  false,
    .local_fs		=  false,
    .set_mask           = 0,
    .del_mask           = 0
  };

  global_args = &args;
  while (1) {
    int		c = getopt_long(argc, argv, CMDLINE_OPTIONS_SHORT,
				CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_IMMU		:  args.set_mask |= VC_IATTR_IMMUTABLE; /*@fallthrough@*/
      case CMD_IMMUX		:  args.set_mask |= VC_IATTR_IUNLINK; break;
      case CMD_ADMIN		:  args.set_mask |= VC_IATTR_ADMIN;   break;
      case CMD_WATCH		:  args.set_mask |= VC_IATTR_WATCH;   break;
      case CMD_HIDE		:  args.set_mask |= VC_IATTR_HIDE;    break;
      case CMD_BARRIER		:  args.set_mask |= VC_IATTR_BARRIER; break;
      case CMD_UNSET_IMMU	:  args.del_mask |= VC_IATTR_IMMUTABLE; /*@fallthrough@*/
      case CMD_UNSET_IMMUX	:  args.del_mask |= VC_IATTR_IUNLINK; break;
      case CMD_UNSET_ADMIN	:  args.del_mask |= VC_IATTR_ADMIN;   break;
      case CMD_UNSET_WATCH	:  args.del_mask |= VC_IATTR_WATCH;   break;
      case CMD_UNSET_HIDE	:  args.del_mask |= VC_IATTR_HIDE;    break;
      case CMD_UNSET_BARRIER	:  args.del_mask |= VC_IATTR_BARRIER; break;
      case 'R'			:  args.do_recurse     = true;  break;
      case 'a'			:  args.do_display_dot = true;  break;
      case 'd'			:  args.do_display_dir = true;  break;
      case 'n'			:  args.do_mapping     = false; break;
      case 's'			:  args.do_set         = true;  break;
      case 'u'			:  args.do_unset       = true;  break;
      case 'c'			:  args.ctx_str        = optarg; break;
      case 'x'			:  args.local_fs       = true;   break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  fixupParams(&args, argc);

  if (optind==argc)
    err_cnt  = processFile(".");
  else for (i=optind; i<argc; ++i)
    err_cnt += processFile(argv[i]);

  return err_cnt>0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
