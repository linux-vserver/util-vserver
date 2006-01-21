// $Id$    --*- c -*--

// Copyright (C) 2006 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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
#include <lib/vserver.h>
#include <lib/fmt.h>

#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#define ENSC_WRAPPERS_PREFIX	"vdu: "
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_DIRENT	1
#define ENSC_WRAPPERS_FCNTL	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001
#define CMD_XID			0x2000
#define CMD_SPACE		0x2001
#define CMD_INODES		0x2002
#define CMD_SCRIPT		0x2003
#define CMD_BLOCKSIZE		0x2005

int			wrapper_exit_code = 1;

struct option const
CMDLINE_OPTIONS[] = {
  { "help",           no_argument,       0, CMD_HELP },
  { "version",        no_argument,       0, CMD_VERSION },
  { "xid",            required_argument, 0, CMD_XID },
  { "space",          no_argument,       0, CMD_SPACE },
  { "inodes",         no_argument,       0, CMD_INODES },
  { "script",         no_argument,       0, CMD_SCRIPT },
  { "blocksize",      required_argument, 0, CMD_BLOCKSIZE },
  {0,0,0,0}
};

struct Arguments {
    xid_t	xid;
    bool	space;
    bool	inodes;
    bool	script;
    uint32_t	blocksize;
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n    ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " --xid <xid> (--space|--inodes) [--blocksize <blocksize>] [--script] <directory>*\n"
	    "\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vdu " VERSION " -- calculates the size of a directory\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2006 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

/* basic hash table implementation for inode tracking */
#define HASH_SIZE 103
typedef struct hash_entry {
  struct hash_entry *next;
  ino_t inode;
} hash_entry;

typedef struct hash_table {
  hash_entry *entries[HASH_SIZE];
} hash_table;

static hash_table ht;

static void
hash_init(void)
{
  memset(&ht, 0, sizeof(hash_table));
}

static void
hash_free(void)
{
  int i;
  hash_entry *e, *p;
  for (i = 0; i < HASH_SIZE; i++) {
    for (e = ht.entries[i], p = NULL; e; e = e->next) {
      free(p);
      p = e;
    }
    free(p);
  }
}

static int
hash_insert(ino_t inode)
{
  hash_entry *e, *p;
  unsigned int hashval = inode % HASH_SIZE;

  /* no one else here */
  if (ht.entries[hashval] == NULL) {
    ht.entries[hashval]        = malloc(sizeof(hash_entry));
    ht.entries[hashval]->next  = NULL;
    ht.entries[hashval]->inode = inode;
    return 0;
  }

  for (e = ht.entries[hashval], p = NULL; e; e = e->next) {
    /* already in the hash table */
    if (e->inode == inode)
      return -1;
    else if (e->inode > inode) {
      /* we're first */
      if (p == NULL) {
	ht.entries[hashval]        = malloc(sizeof(hash_entry));
	ht.entries[hashval]->next  = e;
	ht.entries[hashval]->inode = inode;
      }
      /* we're in the middle */
      else {
	p->next        = malloc(sizeof(hash_entry));
	p->next->next  = e;
	p->next->inode = inode;
      }
      return 0;
    }
    p = e;
  }
  /* we're last */
  p->next        = malloc(sizeof(hash_entry));
  p->next->next  = NULL;
  p->next->inode = inode;

  return 0;
}

static void
vdu_onedir(struct Arguments const *args, char const *path, uint64_t *size)
{
  DIR *dir;
  struct dirent *ent;
  struct stat dirst, st;
  char entpath[PATH_MAX];

  if (lstat(path, &dirst) == -1) {
    WRITE_MSG(2, "lstat(");
    WRITE_STR(2, path);
    WRITE_MSG(2, ")");
    perror("");
    exit(EXIT_FAILURE);
  }

  dir = Eopendir(path);
  while ((ent = Ereaddir(dir)) != NULL) {
    if (ent->d_name[0] == '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
      continue;

    strcpy(entpath, path);
    strcat(entpath, "/");
    strcat(entpath, ent->d_name);

    if (lstat(entpath, &st) == -1) {
      WRITE_MSG(2, "lstat(");
      WRITE_STR(2, entpath);
      WRITE_MSG(2, ")");
      perror("");
      exit(EXIT_FAILURE);
    }

    if (vc_getfilecontext(entpath) != args->xid)
      continue;

    if (st.st_nlink > 1 && hash_insert(st.st_ino) == -1)
      continue;

    if (args->space)
      *size += st.st_blocks << 9; /* * 512 */
    else
      (*size)++;

    if (S_ISDIR(st.st_mode) && dirst.st_dev == st.st_dev)
      vdu_onedir(args, entpath, size);
  }
  Eclosedir(dir);
}

int main(int argc, char *argv[])
{
  struct Arguments		args = {
    .xid       = VC_NOCTX,
    .space     = false,
    .inodes    = false,
    .script    = false,
    .blocksize = 1024,
  };
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_XID	:  args.xid = Evc_xidopt2xid(optarg,true); break;
      case CMD_SPACE	:  args.space = true;                      break;
      case CMD_INODES	:  args.inodes = true;                     break;
      case CMD_SCRIPT	:  args.script = true;                     break;
      case CMD_BLOCKSIZE:
	{
	  char *endptr;
	  args.blocksize = strtol(optarg, &endptr, 0);
	  if ((args.blocksize == 0 && errno != 0) || *endptr != '\0') {
	    WRITE_MSG(2, "Invalid block size argument: '");
	    WRITE_STR(2, optarg);
	    WRITE_MSG(2, "'; try '--help' for more information\n");
	    return EXIT_FAILURE;
	  }
          break;
	}
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return 255;
	break;
    }
  }

  if (args.xid==VC_NOCTX)
    WRITE_MSG(2, "No xid specified; try '--help' for more information\n");
  else if (!args.space && !args.inodes)
    WRITE_MSG(2, "Must specify --space or --inodes; try '--help' for more information\n");
  else if (args.space && args.inodes)
    WRITE_MSG(2, "Can only do one thing at a time; try '--help' for more information\n");
  else if (optind==argc)
    WRITE_MSG(2, "No directory specified; try '--help' for more information\n");
  else {
    int		i;
    uint64_t	size;
    char	buf[sizeof(size)*3 + 2];
    size_t	len;

    for (i = optind; i < argc; i++) {
      size = 0;

      hash_init();
      vdu_onedir(&args, argv[i], &size);
      hash_free();

      if (!args.script) {
	WRITE_STR(1, argv[i]);
	WRITE_MSG(1, " ");
      }
      if (args.space)
        size /= args.blocksize;
      len = utilvserver_fmt_uint64(buf, size);
      Vwrite(1, buf, len);
      WRITE_MSG(1, "\n");
    }
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
