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

#include "pathconfig.h"

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

static void
initMatchList(struct MatchList *list, char const *vserver,
	      char const *vdir, char const *exclude_file)
{
  char			*buf[2] = { 0,0 };
  
  char			**fixed_files = 0;
  size_t		fixed_count   = 0;

  char			**expr_files  = 0;
  size_t		expr_count    = 0;

  if (global_args->verbosity>3) {
    WRITE_MSG(1, "Initializing exclude-list for ");
    WRITE_STR(1, vdir);
    if (vserver!=0) {
      WRITE_MSG(1, " (");
      WRITE_STR(1, vserver);
      WRITE_MSG(1, ")");
    }
    WRITE_MSG(1, "\n");
  }
  if (vserver && global_args->do_renew) {
    if (global_args->verbosity>4)
      WRITE_MSG(1, "  Fetching configuration-file list from packagemanagement\n");
    getConfigfileList(vserver, &fixed_files, &fixed_count, buf+0);
  }

  // abuse special values (NULL, empty string) to skip the next step
  if (exclude_file && *exclude_file) {
    if (global_args->verbosity>4) WRITE_MSG(1, "  Reading exclude file\n");
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

static bool
initMatchListByVserver(struct MatchList *list, char const *vserver,
		       char const **res_appdir)
{
  vcCfgStyle	style;
  char const	*vdir;
  char const 	*appdir;

  style  = vc_getVserverCfgStyle(vserver);
  vdir   = vc_getVserverVdir(  vserver, style, true);
  appdir = vc_getVserverAppDir(vserver, style, "vunify");

  if (vdir==0 || appdir==0) {
    free((char *)appdir);
    free((char *)vdir);
    return false;
  }

  {
    size_t		l1 = strlen(appdir);
    char		tmp[l1 + sizeof("/exclude")];
    char const *	excl_list;

    memcpy(tmp,    appdir, l1);
    memcpy(tmp+l1, "/exclude", 9);

    excl_list = tmp;
    if (access(excl_list, R_OK)==-1) excl_list = CONFDIR   "/.defaults/apps/vunify/exclude";
    if (access(excl_list, R_OK)==-1) excl_list = PKGLIBDIR "/defaults/vunify-exclude";

      // 'vdir' is transferred to matchlist and must not be free'ed here
    initMatchList(list, vserver, vdir, excl_list);
  }

  if (res_appdir!=0)
    *res_appdir = appdir;
  else
    free((char *)appdir);
  
  return true;
}


static void
initModeManually(struct Arguments const UNUSED *args, int argc, char *argv[])
{
  int		i, count=argc/2;
  
  if (argc%2) {
    WRITE_MSG(2, "Odd number of (path,excludelist) arguments\n");
    exit(1);
  }

  if (count<2) {
    WRITE_MSG(2, "No reference path(s) given\n");
    exit(1);
  }

  initMatchList(&global_info.dst_list, 0, argv[0], argv[1]);

  --count;
  global_info.src_lists.v = Emalloc(sizeof(struct MatchList) * count);
  global_info.src_lists.l = count;

  for (i=0; i<count; ++i)
    initMatchList(global_info.src_lists.v+i, 0,
		  strdup(argv[2 + i*2]), argv[3 + i*2]);
}

static int
selectRefserver(struct dirent const *ent)
{
  return strncmp(ent->d_name, "refserver.", 10)==0;
}

static void
initModeVserver(struct Arguments const UNUSED *args, int argc, char *argv[])
{
  char const	*appdir;
  int		cur_dir = Eopen(".", O_RDONLY, 0);
  struct dirent	**entries;
  int		count, i;
  
  if (argc!=1) {
    WRITE_MSG(2, "More than one vserver is not supported\n");
    exit(1);
  }

  if (!initMatchListByVserver(&global_info.dst_list, argv[0], &appdir)) {
    WRITE_MSG(2, "unification not configured for this vserver\n");
    exit(1);
  }

  Echdir(appdir);
  count = scandir(".", &entries, selectRefserver, alphasort);
  if (count==-1) {
    perror("scandir()");
    exit(1);
  }

  if (count==0) {
    WRITE_MSG(2, "no reference vserver configured\n");
    exit(1);
  }

  global_info.src_lists.v = Emalloc(sizeof(struct MatchList) * count);
  global_info.src_lists.l = count;
  for (i=0; i<count; ++i) {
    char const 		*tmp   = entries[i]->d_name;
    size_t		l      = strlen(tmp);
    char		vname[sizeof("./") + l];

    memcpy(vname,   "./", 2);
    memcpy(vname+2, tmp,  l+1);
    
    if (!initMatchListByVserver(global_info.src_lists.v+i, vname, 0)) {
      WRITE_MSG(2, "unification for reference vserver not configured\n");
      exit(1);
    }

    free(entries[i]);
  }
  free(entries);
  free(const_cast(char *)(appdir));

  Efchdir(cur_dir);
  Eclose(cur_dir);
}
