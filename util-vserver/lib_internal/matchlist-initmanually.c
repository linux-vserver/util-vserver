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
#include "pathconfig.h"

#include "matchlist.h"
#include "util-io.h"

#include <wait.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_STDLIB	1
#define ENSC_WRAPPERS_IO	1
#include <wrappers.h>


extern int	Global_getVerbosity() PURE CONST;
extern bool	Global_doRenew() PURE CONST;

static void
readExcludeListFD(int fd,
		  char ***files,  size_t *size,
		  char **buf)
{
  off_t		len;
  size_t	lines = 0;
  char		*ptr;
  
  if (fd==-1) return; // todo: message on verbose?

  len = Elseek(fd, 0, SEEK_END);
  Elseek(fd, 0, SEEK_SET);

  *buf = Emalloc(sizeof(*buf) * (len+1));
  EreadAll(fd, *buf, len);
  (*buf)[len] = '\0';

  ptr = *buf;
  while ((ptr=strchr(ptr, '\n'))) {
    ++lines;
    ++ptr;
  }

  ++lines;
  *files = Emalloc(sizeof(**files) * lines);

  *size = 0;
  ptr   = *buf;
  while (*ptr) {
    char	*end_ptr = strchr(ptr, '\n');

    assert(*size<lines);
    if (end_ptr==0) break;

    if (*ptr!='#') {
      char	*tmp = end_ptr;
      do {
	*tmp-- = '\0';
      } while (tmp>ptr && *tmp==' ');
      
      if (tmp>ptr) (*files)[(*size)++] = ptr;
    }

    ptr = end_ptr+1;
  }
}

static void
readExcludeList(char const *filename,
		char ***files,  size_t *size,
		char **buf)
{
  int		fd = open(filename, O_RDONLY);
  if (fd==-1) return; // todo: message on verbose?

  readExcludeListFD(fd, files, size, buf);
  Eclose(fd);
}

static void
getConfigfileList(char const *vserver,
		  char ***files, size_t *size,
		  char **buf)
{
  char 		tmpname[] = "/tmp/vunify.XXXXXX";
  pid_t		pid;
  int		fd = Emkstemp(tmpname);

  Eunlink(tmpname);
  pid = Efork();

  if (pid==0) {
    char	*args[10];
    char const	**ptr = (char const **)(args)+0;
    
    Edup2(fd, 1);
    //Eclose(0);
    if (fd!=1) Eclose(fd);

    *ptr++  = VPKG_PROG;
    *ptr++  = vserver;
    *ptr++  = "get-conffiles";
    *ptr    = 0;

    Eexecv(args[0], args);
  }
  else {
    int		status;
    
    if (TEMP_FAILURE_RETRY(wait4(pid, &status, 0,0))==-1) {
      perror("wait4()");
      exit(1);
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status)!=0) {
      WRITE_MSG(2, "failed to determine configfiles\n");
      exit(1);
    }

    readExcludeListFD(fd, files, size, buf);
    Eclose(fd);
  }
}

void
MatchList_initManually(struct MatchList *list, char const *vserver,
		       char const *vdir, char const *exclude_file)
{
  char			*buf[2] = { 0,0 };
  
  char			**fixed_files = 0;
  size_t		fixed_count   = 0;

  char			**expr_files  = 0;
  size_t		expr_count    = 0;

  if (Global_getVerbosity()>=1) {
    WRITE_MSG(1, "Initializing exclude-list for ");
    WRITE_STR(1, vdir);
    if (vserver!=0) {
      WRITE_MSG(1, " (");
      WRITE_STR(1, vserver);
      WRITE_MSG(1, ")");
    }
    WRITE_MSG(1, "\n");
  }
  if (vserver && Global_doRenew()) {
    if (Global_getVerbosity()>=2)
      WRITE_MSG(1, "  Fetching configuration-file list from packagemanagement\n");
    getConfigfileList(vserver, &fixed_files, &fixed_count, buf+0);
  }

  // abuse special values (NULL, empty string) to skip the next step
  if (exclude_file && *exclude_file) {
    if (Global_getVerbosity()>=6) WRITE_MSG(1, "  Reading exclude file\n");
    readExcludeList(exclude_file,
		    &expr_files,  &expr_count,
		    buf+1);
  }

  MatchList_init(list, vdir, fixed_count + expr_count);
  list->buf       = Emalloc(sizeof(void *) * 3);
  list->buf[0]    = buf[0];
  list->buf[1]    = buf[1];
  list->buf[2]    = vdir;
  list->buf_count = 3;

  MatchList_appendFiles(list, 0,           fixed_files, fixed_count, false);
  MatchList_appendFiles(list, fixed_count, expr_files,  expr_count,  true);

  free(expr_files);
  free(fixed_files);
}
