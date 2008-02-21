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

static UNUSED void
freeHashList(HashDirCollection *hash_vec)
{
  for (struct HashDirInfo *itm = Vector_begin(hash_vec);
       itm!=Vector_end(hash_vec);
       ++itm) {
    free(const_cast(char *)(itm->path.d));
  }

  Vector_free(hash_vec);
}

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

static bool
initHashMethod(struct HashDirConfiguration *conf, char const *filename)
{
  int		fd = open(filename, O_RDONLY);
  if (fd==-1 && conf->method==0)
    conf->method = ensc_crypto_hash_get_default();

  if (fd==-1) {
    char const	*hash_name;
    assert(conf->method!=0);
    if (conf->method==0)      return false;
    if (global_args->dry_run) return true;	// do not create the file

    fd = Eopen(filename, O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW, 0644);

    hash_name = ensc_crypto_hash_get_name(conf->method);
    TEMP_FAILURE_RETRY(write(fd, hash_name, strlen(hash_name)));
    TEMP_FAILURE_RETRY(write(fd, "\n", 1));
  }
  else {
    off_t	s  = Elseek(fd, 0, SEEK_END);
    char	buf[s + 1];
    Elseek(fd, 0, SEEK_SET);

    conf->method=0;

    if (s>0 && read(fd, buf, s+1)==s) {
      while (s>0 && (buf[s-1]=='\0' || buf[s-1]=='\n'))
	--s;
      buf[s] = '\0';

      conf->method = ensc_crypto_hash_find(buf);
      if (conf->method==0) {
	WRITE_MSG(2, "Can not find hash-function '");
	WRITE_STR(2, buf);
	WRITE_MSG(2, "'\n");
      }
    }
    else
      WRITE_MSG(2, "Can not read configuration file for hash-method\n");
  }

  if (conf->method!=0 && ensc_crypto_hash_get_digestsize(conf->method)*8>HASH_MAXBITS) {
    WRITE_MSG(2, "Wow... what an huge hash-function. I can not handle so much bits; giving up...\n");
    conf->method=0;
  }
  
  Eclose(fd);
  return conf->method!=0;
}

static bool
initHashBlocks(struct HashDirConfiguration *conf, char const *filename)
{
  int		fd = open(filename, O_RDONLY);

  if (fd==-1) {
    char		str[sizeof("all,start,middle,end,")] = { [0] = '\0' };

    if (global_args->dry_run) return true;	// do not create the file
    
    fd = Eopen(filename, O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW, 0644);

    if (conf->blocks== hshALL)    strcat(str, "all\n");
    if (conf->blocks & hshSTART)  strcat(str, "start\n");
    if (conf->blocks & hshMIDDLE) strcat(str, "middle\n");
    if (conf->blocks & hshEND)    strcat(str, "end\n");

    EwriteAll(fd, str, strlen(str));
  }
  else {
    off_t	s  = Elseek(fd, 0, SEEK_END);
    char	buf[s + 1];
    Elseek(fd, 0, SEEK_SET);
    
    conf->blocks = hshINVALID;
      
    if (s>0 && read(fd, buf, s+1)==s) {
      char	*tok = buf;
      char	*sep = "\n,\t ";

      buf[s]       = '\0';
      conf->blocks = hshALL;

      do {
	char	*ptr = strsep(&tok, sep);

	if (*ptr=='#') { sep = "\n"; continue; }
	sep = "\n,\t ";
	if (*ptr=='\0') continue;

	if      (strcasecmp(ptr, "all")   ==0) conf->blocks  = hshALL;
	else {
	  if (conf->blocks==hshINVALID) conf->blocks = 0;
	  
	  else if (strcasecmp(ptr, "start") ==0) conf->blocks |= hshSTART;
	  else if (strcasecmp(ptr, "middle")==0) conf->blocks |= hshMIDDLE;
	  else if (strcasecmp(ptr, "end")   ==0) conf->blocks |= hshEND;
	  else {
	    WRITE_MSG(2, "Invalid block descriptor '");
	    WRITE_STR(2, ptr);
	    WRITE_MSG(2, "'\n");
	    conf->blocks = hshINVALID;
	    tok = 0;
	  }
	}
      } while (tok!=0);
    }
    else
      WRITE_MSG(2, "Can not read configuration file for hash-blocks\n");
  }

  Eclose(fd);
  return conf->blocks!=hshINVALID;
}

static bool
initHashBlockSize(struct HashDirConfiguration *conf, char const *filename)
{
  if (conf->blocks==hshALL) return true;
  
  int		fd = open(filename, O_RDONLY);
  if (fd==-1) {
    char		str[sizeof("0x") + sizeof(size_t)*3+2] = {
      [0] = '0', [1] = 'x'
    };
    size_t		len = utilvserver_fmt_xuint(str+2, conf->blocksize);

    if (global_args->dry_run) return true;	// do not create the file

    fd = Eopen(filename, O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW, 0644);
    EwriteAll(fd, str, len+2);
  }
  else {
    off_t	s  = Elseek(fd, 0, SEEK_END);
    char	buf[s + 1];
    Elseek(fd, 0, SEEK_SET);

    conf->blocksize = (size_t)(-1);

    if (s>0 && read(fd, buf, s+1)==s) {
      char	*errptr;
      
      while (s>0 && (buf[s-1]=='\0' || buf[s-1]=='\n'))
	--s;
      buf[s] = '\0';

      conf->blocksize = strtol(buf, &errptr, 0);
      if (errptr==buf || (*errptr!='\0' && *errptr!='\n')) {
	WRITE_MSG(2, "Failed to parse blocksize '");
	WRITE_STR(2, buf);
	WRITE_MSG(2, "'\n");
	conf->blocksize = (size_t)(-1);
      }
    }
    else
      WRITE_MSG(2, "Can not read configuration file for hash-blocksize\n");
  }

  Eclose(fd);
  return conf->blocksize!=(size_t)(-1);
}

static bool
initHashConf(struct HashDirConfiguration *conf, char const *hashdir)
{
  size_t		l = strlen(hashdir);
  char			tmp[l + MAX(MAX(sizeof("/method"), sizeof("/blocks")),
				    sizeof("/blocksize"))];

  memcpy(tmp, hashdir, l);

  return ((strcpy(tmp+l, "/method"),    initHashMethod   (conf, tmp)) &&
	  (strcpy(tmp+l, "/blocks"),    initHashBlocks   (conf, tmp)) &&
	  (strcpy(tmp+l, "/blocksize"), initHashBlockSize(conf, tmp)));
}

static char *
searchHashdir(char const *lhs, char const *rhs)
{
  size_t	l1  = strlen(lhs);
  size_t	l2  = rhs ? strlen(rhs) : 0;
  char *	res = Emalloc(l1 + l2 + 1);
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

  if (!initHashConf(&global_info.hash_conf, args->hash_dir)) {
    WRITE_MSG(2, "failed to initialize hash-configuration\n");
    exit(1);
  }
  
  global_info.hash_dirs_max_size = initHashList(&global_info.hash_dirs,
						args->hash_dir);
  MatchList_initManually(&global_info.dst_list, 0, strdup(argv[0]), argv[1]);
}

static void
initModeVserver(struct Arguments const UNUSED *args, int argc, char *argv[])
{
  char const				*hashdir   = args->hash_dir;
  struct MatchVserverInfo		vserver = {
    .name        = argv[0],
    .use_pkgmgmt = true
  };

  if (!MatchVserverInfo_init(&vserver)) {
    WRITE_MSG(2, "Failed to initialize unification for vserver\n");
    exit(1);
  }

  if (argc!=1) {
    WRITE_MSG(2, "More than one vserver is not supported\n");
    exit(1);
  }

  if (!MatchList_initByVserver(&global_info.dst_list, &vserver)) {
    WRITE_MSG(2, "unification not configured for this vserver\n");
    exit(1);
  }

  if (hashdir==0) hashdir = searchHashdir(vserver.appdir.d, "/hash");
  if (hashdir==0) hashdir = searchHashdir(CONFDIR "/.defaults/apps/vunify/hash", 0);

  if (hashdir==0) {
    WRITE_MSG(2, "no hash-directory configured for this vserver.\n");
    exit(1);
  }

  if (!initHashConf(&global_info.hash_conf, hashdir)) {
    WRITE_MSG(2, "failed to initialize hash-configuration\n");
    exit(1);
  }
  
  global_info.hash_dirs_max_size = initHashList(&global_info.hash_dirs, hashdir);

  free(const_cast(char *)(hashdir));
  MatchVserverInfo_free(&vserver);
}
		
