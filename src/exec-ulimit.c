// $Id$    --*- c -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "util.h"

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define ENSC_WRAPPERS_PREFIX	"exec-ulimit: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_RESOURCE	1
#include <wrappers.h>

/* dietlibc specifies this as (~0UL>>1), which is what's returned from
 * sys_old_getrlimit, called on some arches for getrlimit.
 * Reset it here so the kernel will have the correct values when we set it. */
#undef RLIM_INFINITY
#define RLIM_INFINITY (~0UL)
#define OLD_RLIM_INFINITY (~0UL>>1)

#define DECLARE_LIMIT(RES,FNAME) { #FNAME, RLIMIT_##RES }

int	wrapper_exit_code = 255;

static struct {
    char const	*fname;
    int		code;
} const LIMITS[] = {
  DECLARE_LIMIT(CORE,    core),
  DECLARE_LIMIT(CPU,     cpu),
  DECLARE_LIMIT(DATA,    data),
  DECLARE_LIMIT(FSIZE,   fsize),
  DECLARE_LIMIT(LOCKS,   locks),
  DECLARE_LIMIT(MEMLOCK, memlock),
  DECLARE_LIMIT(NOFILE,  nofile),
  DECLARE_LIMIT(NPROC,   nproc),
  DECLARE_LIMIT(RSS,     rss),
  DECLARE_LIMIT(STACK,   stack),
};

static rlim_t
readValue(int fd, char const *filename)
{
  char		buf[128];
  size_t	len = Eread(fd, buf, sizeof(buf)-1);
  long int	res;
  char *	errptr;

  buf[len] = '\0';
  if (strncmp(buf, "inf", 3)==0) return RLIM_INFINITY;
  res = strtol(buf, &errptr, 0);

  if (errptr!=buf) {
    switch (*errptr) {
      case 'M'	:  res *= 1024; /* fallthrough */
      case 'K'	:  res *= 1024; ++errptr; break;
      case 'm'	:  res *= 1000; /* fallthrough */
      case 'k'	:  res *= 1000; ++errptr; break;
      default	:  break;
    }
  }

  if (errptr==buf || (*errptr!='\0' && *errptr!='\n')) {
    WRITE_MSG(2, "Invalid limit in '");
    WRITE_STR(2, filename);
    WRITE_STR(2, "'\n");
    exit(255);
  }

  return res;
}

static bool
readSingleLimit(struct rlimit *lim, char const *fname_base)
{
  size_t	fname_len = strlen(fname_base);
  char		fname[fname_len + sizeof(".hard")];
  int		fd;
  bool		is_modified = false;

  strcpy(fname, fname_base);
  
  fd = open(fname, O_RDONLY);
  if (fd!=-1) {
    rlim_t	tmp = readValue(fd, fname_base);
    lim->rlim_cur = tmp;
    lim->rlim_max = tmp;
    Eclose(fd);

    is_modified = true;
  }

  strcpy(fname+fname_len, ".hard");
  fd = open(fname, O_RDONLY);
  if (fd!=-1) {
    lim->rlim_max = readValue(fd, fname_base);
    Eclose(fd);
    
    is_modified = true;
  }

  strcpy(fname+fname_len, ".soft");
  fd = open(fname, O_RDONLY);
  if (fd!=-1) {
    lim->rlim_cur = readValue(fd, fname_base);
    Eclose(fd);
    
    is_modified = true;
  }

  if (is_modified &&
      lim->rlim_max!=RLIM_INFINITY &&
      (lim->rlim_cur==RLIM_INFINITY ||
       lim->rlim_cur>lim->rlim_max))
    lim->rlim_cur = lim->rlim_max;

  return is_modified;
}

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_STR(fd,
	    "<ulimit-cfgdir> <cmd> <argv>*\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "exec-ulimit " VERSION " -- executes programs with resource limits\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int main(int argc, char *argv[])
{
  size_t		i;
  int			cur_fd = Eopen(".", O_RDONLY, 0);
  bool			in_dir = false;

  if (argc==2) {
    if (strcmp(argv[1], "--help")==0)    showHelp(1,argv[0],0);
    if (strcmp(argv[1], "--version")==0) showVersion();
  }

  if (argc<3) {
    WRITE_MSG(2, "Bad parameter count; use '--help' for more information.\n");
    exit(255);
  }

  if (chdir(argv[1])!=-1) {
    in_dir = true;
  }
  for (i=0; i<sizeof(LIMITS)/sizeof(LIMITS[0]); ++i) {
    struct rlimit	limit;

    Egetrlimit(LIMITS[i].code, &limit);
    /* if this arch uses sys_old_getrlimit... */
    if (limit.rlim_cur == OLD_RLIM_INFINITY)
      limit.rlim_cur = RLIM_INFINITY;
    if (in_dir && readSingleLimit(&limit, LIMITS[i].fname))
      Esetrlimit(LIMITS[i].code, &limit);
    else if (LIMITS[i].code != RLIMIT_NOFILE) {
      limit.rlim_max = RLIM_INFINITY;
      Esetrlimit(LIMITS[i].code, &limit);
    }
    else {
      /* RLIMIT_NOFILE can't be set to infinity, 1024*1024 seems to be the limit in most kernels */
      limit.rlim_max = 1024*1024;
      setrlimit(LIMITS[i].code, &limit);
    }
  }
  if (in_dir)
    Efchdir(cur_fd);
  Eclose(cur_fd);

  Eexecv(argv[2], argv+2);
}
