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
#include "wrappers.h"
#include "wrappers-dirent.h"

#include <lib/vserver.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>


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
  return (d[0]=='.' && (d[1]=='\0' || (d[1]=='.' && d[2]=='\0')));
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
  DIR *			dir = Eopendir(".");
  uint64_t		err = 0;

  {
    struct stat		st;
    if (lstat(".", &st)==-1) perror("lstat()");
    else err += handleFile(".", path, &st) ? 0 : 1;
  }

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
      err += handleFile(ent->d_name, new_path, &st) ? 0 : 1;
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

      if (!S_ISDIR(st.st_mode)) continue;
      safeChdir(ent->d_name, &st);
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

  if (S_ISDIR(st.st_mode) && !global_args->do_display_dir)
    return iterateFilesystem(path);
  else
    return handleFile(path, path, &st);
}

static xid_t
resolveCtx(char const *str)
{
  xid_t		res;
  
  if (*str==':') ++str;
  else {
    char	*end_ptr;
    long	result = strtol(str, &end_ptr, 0);

    if (end_ptr>str && *end_ptr==0) return result;
  }

  res = vc_getVserverCtx(str, vcCFG_AUTO, true, 0);
  if (res==VC_NOCTX) {
    WRITE_MSG(2, "Can not find a vserver with name '");
    WRITE_STR(2, str);
    WRITE_MSG(2, "', or vserver does not have a static context\n");
    exit(1);
  }

  return res;
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
    .immutable		=  false,
    .immulink		=  false,
    .ctx		=  VC_NOCTX,
    .is_legacy          =  false,
    .do_set             =  false,
    .do_unset           =  false,
  };

  global_args = &args;
  while (1) {
    int		c = getopt_long(argc, argv, CMDLINE_OPTIONS_SHORT,
				CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_IMMUTABLE	:  args.immutable = true; break;
      case CMD_IMMULINK		:  args.immulink  = true; break;
      case CMD_LEGACY		:  args.is_legacy      = true;  break;
      case 'R'			:  args.do_recurse     = true;  break;
      case 'a'			:  args.do_display_dot = true;  break;
      case 'd'			:  args.do_display_dir = true;  break;
      case 'n'			:  args.do_mapping     = false; break;
      case 's'			:  args.do_set         = true;  break;
      case 'u'			:  args.do_unset       = true;  break;
      case 'c'			:  args.ctx            = resolveCtx(optarg); break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  checkParams(&args, argc);

  if (optind==argc)
    err_cnt  = processFile(".");
  else for (i=optind; i<argc; ++i)
    err_cnt += processFile(argv[i]);

  return err_cnt>0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
