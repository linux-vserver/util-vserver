// $Id$    --*- c++ -*--

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


  // secure-mount <general mount(8) options> [--chroot]
  //              [--mtab <mtabfile>] [--fstab <fstabfile>]
  //
  // Executes mount-operations under the current directory: it assumes sources
  // in the current root-dir while destinations are expected in the chroot
  // environment.


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util.h"
#include "pathconfig.h"

#include <lib/internal.h>

#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <linux/fs.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>
#include <libgen.h>
#include <signal.h>
#include <stdlib.h>

#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

#define MNTPOINT	"/etc"

typedef enum { rfsYES, rfsNO, rfsONLY }		RootFsOption;

struct MountInfo {
    char const *	src;
    char const *	dst;
    char const *	type;
    unsigned long	flag;
    unsigned long	xflag;
    unsigned long	mask;
    char *		data;
    char *		data_parsed;
};

struct Options {
    char const *	mtab;
    char const *	fstab;
    bool		do_chroot;
    bool		ignore_mtab;
    bool		mount_all;
    RootFsOption	rootfs;

    int			cur_dir_fd;
    int			cur_rootdir_fd;
};

#define OPTION_BIND	1024
#define OPTION_MOVE	1025
#define OPTION_MTAB	1026
#define OPTION_FSTAB	1027
#define OPTION_CHROOT	1028
#define OPTION_SECURE	1029
#define OPTION_RBIND	1030
#define OPTION_ROOTFS	1031

#define XFLAG_NOAUTO	0x01

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",    no_argument,       0, 'h' },
  { "version", no_argument,       0, 'v' },
  { "bind",    no_argument,       0, OPTION_BIND },
  { "move",    no_argument,       0, OPTION_MOVE },
  { "mtab",    required_argument, 0, OPTION_MTAB },
  { "fstab",   required_argument, 0, OPTION_FSTAB },
  { "rootfs",  required_argument, 0, OPTION_ROOTFS },
  { "chroot",  no_argument,	  0, OPTION_CHROOT },
  { "secure",  no_argument,       0, OPTION_SECURE },
  { "rbind",   no_argument,       0, OPTION_RBIND },
  { 0, 0, 0, 0 }
};

#ifndef MS_REC
#  define MS_REC	0x4000
#endif

static struct FstabOption {
    char const * const	opt;
    unsigned long const		flag;
    unsigned long const		mask;
    unsigned long const 	xflag;
    bool const			is_dflt;
} const FSTAB_OPTIONS[] = {
  { "defaults",   MS_NODEV,      (MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC|
				  MS_SYNCHRONOUS), 0, false },
  { "rbind",      MS_BIND|MS_REC, MS_BIND|MS_REC,  0, false },
  { "bind",       MS_BIND,        MS_BIND,         0, false },
  { "move",       MS_MOVE,        MS_MOVE,         0, false },
  { "async",      0,              MS_SYNCHRONOUS,  0, false },
  { "sync",       MS_SYNCHRONOUS, MS_SYNCHRONOUS,  0, false },
  { "atime",      0,              MS_NOATIME,      0, false },
  { "noatime",    MS_NOATIME,     MS_NOATIME,      0, false },
  { "dev",        0,              MS_NODEV,        0, false },
  { "nodev",      MS_NODEV,       MS_NODEV,        0, false },
  { "exec",       0,              MS_NOEXEC,       0, false },
  { "noexec",     MS_NOEXEC,      MS_NOEXEC,       0, false },
  { "suid",       0,              MS_NOSUID,       0, false },
  { "nosuid",     MS_NOSUID,      MS_NOSUID,       0, false },
  { "ro",         MS_RDONLY,      MS_RDONLY,       0, false },
  { "rw",         0,              MS_RDONLY,       0, false },
  
  { "remount",    MS_REMOUNT,     MS_REMOUNT,      0, false },
  { "users",      MS_NOEXEC|MS_NOSUID|MS_NODEV,
                  MS_NOEXEC|MS_NOSUID|MS_NODEV,    0, false },
  { "mandlock",   MS_MANDLOCK,    MS_MANDLOCK,     0, false },
  { "nodiratime", MS_NODIRATIME,  MS_NODIRATIME,   0, false },
#ifdef MS_DIRSYNC  
  { "dirsync",    MS_DIRSYNC,     MS_DIRSYNC,      0, false },
#endif
  { "_netdev",    0,              0,               0, false },
  { "auto",       0,              0,               0, false },
  { "noauto",     0,              0,               XFLAG_NOAUTO, false },
  { "user",       0,              0,               0, false },
  { "nouser",     0,              0,               0, false },
};

int			wrapper_exit_code = 1;

static void
showHelp(int fd, char const *cmd, int res)
{
  VSERVER_DECLARE_CMD(cmd);
  
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--help] [--version] [--bind] [--move] [--rbind] [-t <type>] [--chroot]\n"
	    "            [--mtab <filename>] [--fstab <filename>] [--rootfs yes|no|only]\n"
	    "            [-n] -a|([-o <options>] [--] <src> <dst>)\n\n"
	    "Executes mount-operations under the current directory: it assumes sources in\n"
	    "the current root-dir while destinations are expected in the chroot environment.\n\n"
	    "For non-trivial mount-operations it uses the external 'mount' program which\n"
	    "can be overridden by the $MOUNT environment variable.\n\n"
	    "Options:\n"
            "  --bind|move|rbind        ...  set the correspond flags; with this options\n"
            "                                the mount will be executed internally without\n"
            "                                calling an external mount program.\n"
            "  -t <type>                ...  assume the given filesystem type\n"
            "  -o <options>             ...  set additional options; see mount(2) for details\n"
            "  -n                       ...  do not update the mtab-file\n"
            "  --mtab <filename>        ...  use <filename> as an alternative mtab file\n"
            "                                [default: /etc/mtab]\n"
            "  --chroot                 ...  chroot into the current directory before\n"
            "                                mounting the filesystem\n"
            "  --fstab <filename>       ...  use <filename> as an alternative fstab file;\n"
            "                                this option has an effect only with the '-a'\n"
            "                                option [default: /etc/fstab]\n"
	    "  --rootfs yes|no|only     ...  specifies how to handle an entry for a rootfs\n"
	    "                                ('/') when processing an fstab file. 'yes' will\n"
	    "                                mount it among the other entries, 'only' will\n"
	    "                                mount only the rootfs entry, and 'no' will ignore\n"
	    "                                it and mount only the other entries [default: yes]\n"
            "  -a                       ...  mount everything listed in the fstab-file\n\n"
            "  <src>                    ...  the source-filesystem; this path is absolute\n"
            "                                to the current root-filesystem. Only valid\n"
            "                                without the '-a' option.\n"
            "  <dst>                    ...  the destination mount-point; when used with\n"
            "                                '--chroot', this path is relative to the current\n"
            "                                directory. Only valid without the '-a' option\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "secure-mount " VERSION " -- secure mounting of directories\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

inline static bool
isSameObject(struct stat const *lhs,
	     struct stat const *rhs)
{
  return (lhs->st_dev==rhs->st_dev &&
	  lhs->st_ino==rhs->st_ino);
}

static int
fchroot(int fd)
{
  if (fchdir(fd)==-1 || chroot(".")==-1) return -1;
  return 0;
}

static int
writeX(int fd, void const *buf, size_t len)
{
  if ((size_t)(write(fd, buf, len))!=len) return -1;
  return 0;
}

static int
writeStrX(int fd, char const *str)
{
  return writeX(fd, str, strlen(str));
}

static inline char const *
getType(struct MountInfo const *mnt)
{
  if (mnt->type==0)                         return "none";
  else if (strncmp(mnt->type, "ext", 3)==0) return "ufs";
  else                                      return mnt->type;
}

inline static void
restoreRoot(struct Options const *opt)
{
  if (opt->do_chroot!=0 && fchroot(opt->cur_rootdir_fd)==-1) {
    perror("secure-mount: fchdir(\"/\")");
    WRITE_MSG(2, "Failed to restore root-directory; aborting\n");
    exit(1);
  }
}

static int
updateMtab(struct MountInfo const *mnt, struct Options const *opt)
{
  int		res = -1;
  int		fd;
  assert(opt->mtab!=0);

  if (opt->do_chroot && fchroot(opt->cur_dir_fd)==-1) {
      perror("secure-mount: fchroot(\".\")");
      return -1;
  }

  fd=open(opt->mtab, O_CREAT|O_APPEND|O_WRONLY, 0644);
  
  if (fd==-1) {
    perror("secure-mount: open(<mtab>)");
    goto err0;
  }

  if (flock(fd, LOCK_EX)==-1) {
    perror("secure-mount: flock()");
    goto err1;
  }

  if (writeStrX(fd, mnt->src)==-1 ||
      writeStrX(fd, " ")==-1 ||
      writeStrX(fd, mnt->dst)==-1 ||
      writeStrX(fd, " ")==-1 ||
      writeStrX(fd, getType(mnt))==-1 ||
      writeStrX(fd, " ")==-1 ||
      writeStrX(fd, mnt->data ? mnt->data : "defaults")==-1 ||
      writeStrX(fd, " 0 0\n")==-1) {
    perror("secure-mount: write()");
    goto err1;
  }

  res = 0;

  err1:	close(fd);
  err0:
  restoreRoot(opt);
  return res;
}

static bool
callExternalMount(struct MountInfo const *mnt)
{
  char const * 	argv[10];
  size_t	idx = 0;
  pid_t		pid;
  int		status;
  char const *	mount_prog = getenv("MOUNT");

  if (mount_prog==0) mount_prog = MOUNT_PROG;

  argv[idx++] = mount_prog;
  argv[idx++] = "-n";
  if      (mnt->flag & MS_BIND) argv[idx++] = "--bind";
  else if (mnt->flag & MS_MOVE) argv[idx++] = "--move";

  argv[idx++] = "-o";
  if (mnt->data && *mnt->data &&
      strcmp(mnt->data, "defaults")!=0) {
    if (mnt->mask & MS_NODEV)
      argv[idx++] = mnt->data;
    else {
      char *	tmp = alloca(strlen(mnt->data) + sizeof("nodev,"));
      strcpy(tmp, "nodev,");
      strcat(tmp, mnt->data);
      argv[idx++] = tmp;
    }
  }
  else
    argv[idx++] = "nodev";

  if (mnt->type) {
    argv[idx++] = "-t";
    argv[idx++] = mnt->type;
  }

  argv[idx++] = mnt->src;
  argv[idx++] = ".";
  argv[idx]   = 0;

  pid = fork();
  if (pid==-1) {
    perror("secure-mount: fork()");
    return false;
  }

  if (pid==0) {
    execv(mount_prog, const_cast(char **)(argv));
    PERROR_Q("secure-mount: execv", mount_prog);
    exit(1);
  }

  if (wait4(pid, &status, 0, 0)==-1) {
    perror("secure-mount: wait4()");
    return false;
  }

  return (WIFEXITED(status)) && (WEXITSTATUS(status)==0);
}

inline static bool
secureChdir(char const *dir, struct Options const *opt)
{
  int		dir_fd;
  bool		res = false;
  
  if (opt->do_chroot!=0 && fchroot(opt->cur_dir_fd)==-1) {
    perror("secure-mount: fchroot(\".\")");
    return false;
  }

  if (chdir(dir)==-1) {
    PERROR_Q("secure-mount: chdir", dir);
    goto err;
  }

  dir_fd = open(".", O_RDONLY|O_DIRECTORY);
  if (dir_fd==-1) {
    perror("secure-mount: open(\".\")");
    goto err;
  }

  restoreRoot(opt);
  if (fchdir(dir_fd)==-1)
    PERROR_Q("secure-mount: fchdir", dir);
  else
    res = true;
  
  close(dir_fd);
  return res;

  err:
  restoreRoot(opt);
  return false;
}

static bool
canHandleInternal(struct MountInfo const *mnt)
{
  static char const *	FS[] = {
    "tmpfs", "sysfs", "proc", "sockfs", "pipefs", "futexfs",
    "inotifyfs", "devpts", "ext3", "ext2", "ramfs",
    "hugetlbfs", "usbfs", "binfmt_misc",
    0
  };
  char const **		i;
  
  if (!mnt)                                 return false;
  else if ((mnt->flag & (MS_BIND|MS_MOVE))) return true;
  else if (mnt->type==0)                    return false;

  for (i=FS+0; *i!=0; ++i)
    if (strcmp(mnt->type, *i)==0) return true;

  return false;
}

static bool
mountSingle(struct MountInfo const *mnt, struct Options const *opt)
{
  assert(mnt->dst!=0);
  
  if (!secureChdir(mnt->dst, opt))
    return false;

  if (canHandleInternal(mnt)) {
    if (mount(mnt->src, ".",
	      mnt->type ? mnt->type : "",
	      mnt->flag,  mnt->data_parsed)==-1) {
      perror("secure-mount: mount()");
      return false;
    }
  }
  else if (!callExternalMount(mnt))
    return false;

  if (!opt->ignore_mtab &&
      updateMtab(mnt, opt)==-1) {
    WRITE_MSG(2, "Failed to update mtab-file\n");
      // no error
  }
  
  return true;
}

static struct FstabOption const *
searchOption(char const *opt, size_t len)
{
  struct FstabOption const *		i;
  for (i=FSTAB_OPTIONS+0; i<FSTAB_OPTIONS+DIM_OF(FSTAB_OPTIONS); ++i)
    if (strncmp(i->opt, opt, len)==0) return i;

  return 0;
}

static bool
transformOptionList(struct MountInfo *info, size_t UNUSED *col)
{
  char const *			ptr = info->data;
  char *			data = malloc(strlen(info->data));
  char *			dst = data;

  do {
    char const *		pos = strchr(ptr, ',');
    struct FstabOption const *	opt;
    
    if (pos==0) pos = ptr+strlen(ptr);
    opt = searchOption(ptr, pos-ptr);

    if (opt!=0) {
      info->flag  &= ~opt->mask;
      info->flag  |=  opt->flag;
      info->mask  |=  opt->mask;
      info->xflag |=  opt->xflag;
    }
    else {
      if (dst != data)
        *(dst++) = ',';
      strncpy(dst, ptr, pos-ptr);
      dst += pos - ptr;
      *dst = '\0';
    }

    if (*pos!='\0')
      ptr = pos+1;
    else
      ptr = pos;

  } while (*ptr!='\0');

  info->data_parsed = data;
  return true;
}

#define MOVE_TO_NEXT_FIELD(PTR,ALLOW_EOL)		\
  while (!isspace(*PTR) && *PTR!='\0') ++PTR;		\
  if (col) *col = buf-start_buf+1;			\
  if (!(ALLOW_EOL) && *PTR=='\0') return prFAIL;	\
  *PTR++ = '\0';					\
  while (isspace(*PTR)) ++PTR

static enum {prDOIT, prFAIL, prIGNORE}
  parseFstabLine(struct MountInfo *info, char *buf, size_t *col)
{
  char const * const	start_buf = buf;
  size_t		err_col;

  while (isspace(*buf)) ++buf;
  if (*buf=='#' || *buf=='\0')  return prIGNORE;

  info->src  = buf;
  MOVE_TO_NEXT_FIELD(buf, false);
  info->dst  = buf;
  MOVE_TO_NEXT_FIELD(buf, false);
  info->type = buf;
  MOVE_TO_NEXT_FIELD(buf, false);
  err_col    = buf-start_buf+1;
  info->data = buf;
  MOVE_TO_NEXT_FIELD(buf, true);

  info->flag  = MS_NODEV;
  info->mask  = 0;
  info->xflag = 0;

  if      (strcmp(info->type, "swap")  ==0) return prIGNORE;
  else if (strcmp(info->type, "none")  ==0) info->type  = 0;
  else if (strcmp(info->type, "devpts")==0) {
    info->mask |=  MS_NODEV;
    info->flag &= ~MS_NODEV;
  }

  if (col) *col = err_col;
  if (!transformOptionList(info,col)) return prFAIL;
  if (info->xflag & XFLAG_NOAUTO)     return prIGNORE;

  return prDOIT;
}

#undef MOVE_TO_NEXT_FIELD

static void
showFstabPosition(int fd, char const *fname, size_t line_nr, size_t col_nr)
{
  char		buf[3*sizeof(line_nr)*2 + 4];
  size_t	len = utilvserver_fmt_uint(buf+1, line_nr)+1;
  
  buf[0]     = ':';
  buf[len++] = ':';
  len += utilvserver_fmt_uint(buf+len, col_nr);
  WRITE_STR(fd, fname);
  Vwrite(fd, buf, len);
}


static bool
mountFstab(struct Options const *opt)
{
  bool		res = false;
  int		fd;
  off_t		len;

  assert(opt->fstab!=0);
  fd = open(opt->fstab, O_RDONLY);
  if (fd==-1) {
    perror("secure-mount: open(<fstab>)");
    goto err0;
  }

  len = lseek(fd, 0, SEEK_END); 
  if (len==-1 ||
      lseek(fd, 0, SEEK_SET)==-1) {
    perror("secure-mount: lseek(<fstab>)");
    goto err1;
  }

  {
    char	buf[len+2];
    char	*ptr, *ptrptr;
    size_t	line_nr=0, col_nr;

    if (read(fd, buf, len+1)!=len) {
      perror("secure-mount: read()");
      goto err1;
    }
    buf[len]   = '#';	// workaround for broken dietlibc strtok_r()
			// implementation
    buf[len+1] = '\0';
    ptrptr     = buf;

    while ((ptr=strsep(&ptrptr, "\n")) != 0) {
      struct MountInfo	mnt;
      ++line_nr;

      switch (parseFstabLine(&mnt, ptr, &col_nr)) {
	case prFAIL	:
	  showFstabPosition(2, opt->fstab, line_nr, col_nr);
	  WRITE_MSG(2, ": syntax error\n");
	  goto err1;

	case prIGNORE	:  break;
	case prDOIT	: {
	  bool		is_rootfs = (strcmp(mnt.dst, "/")==0);
	  Echdir("/");
	  if (( is_rootfs && opt->rootfs==rfsNO) ||
	      (!is_rootfs && opt->rootfs==rfsONLY)) { /* ignore the entry */ }
	  else if (!mountSingle(&mnt, opt)) {
	    showFstabPosition(2, opt->fstab, line_nr, 1);
	    WRITE_MSG(2, ": failed to mount fstab-entry\n");
	  }
	  break;
	}
	default		:
	  assert(false);
      }
    }
  }

  res = true;

  err1: close(fd);
  err0: return res;
}

static void
initFDs(struct Options *opt)
{
  opt->cur_dir_fd     = Eopen(".", O_RDONLY|O_DIRECTORY, 0);
  opt->cur_rootdir_fd = Eopen("/", O_RDONLY|O_DIRECTORY, 0);

  Efcntl(opt->cur_dir_fd,     F_SETFD, FD_CLOEXEC);
  Efcntl(opt->cur_rootdir_fd, F_SETFD, FD_CLOEXEC);
}

static RootFsOption
parseRootFS(char const *str)
{
  if      (strcasecmp(str, "yes")==0)  return rfsYES;
  else if (strcasecmp(str, "no")==0)   return rfsNO;
  else if (strcasecmp(str, "only")==0) return rfsONLY;
  else {
    WRITE_MSG(2, "secure-mount: invalid option for '--rootfs': '");
    WRITE_STR(2, str);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }
}

int main(int argc, char *argv[])
{
  struct MountInfo	mnt = {
    .src         = 0,
    .dst         = 0,
    .type        = 0,
    .flag        = MS_NODEV,
    .xflag	 = 0,
    .data        = 0,
  };

  struct Options	opt = {
    .mtab           = "/etc/mtab",
    .fstab          = "/etc/fstab",
    .do_chroot      = 0,
    .ignore_mtab    = false,
    .mount_all      = false,
    .cur_dir_fd     = -1,
    .cur_rootdir_fd = -1,
    .rootfs         = rfsYES
  };

  while (1) {
    int		c = getopt_long(argc, argv, "ht:nao:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;
    
    switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case 'v'		:  showVersion();
      case 't'		:  mnt.type = optarg;         break;
      case 'n'		:  opt.ignore_mtab = true;    break;
      case 'a'		:  opt.mount_all   = true;    break;
      case 'o'		:  mnt.data        = optarg;  break;
      case OPTION_RBIND	:  mnt.flag       |= MS_REC;  /*@fallthrough@*/
      case OPTION_BIND	:  mnt.flag       |= MS_BIND; break;
      case OPTION_MOVE	:  mnt.flag       |= MS_MOVE; break;
      case OPTION_MTAB	:  opt.mtab        = optarg;  break;
      case OPTION_FSTAB	:  opt.fstab       = optarg;  break;
      case OPTION_CHROOT:  opt.do_chroot   = true;    break;
      case OPTION_ROOTFS:  opt.rootfs      = parseRootFS(optarg); break;
      case OPTION_SECURE:
	WRITE_MSG(2, "secure-mount: The '--secure' option is deprecated...\n");
	break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }


  if (opt.mount_all && optind<argc) {
    WRITE_MSG(2, "Can not specify <src> and '-a' at the same time\n");
    return EXIT_FAILURE;
  }

  initFDs(&opt);
  signal(SIGCHLD, SIG_DFL);
  
  if (opt.mount_all) {
    if (!mountFstab(&opt)) return EXIT_FAILURE;
    else                   return EXIT_SUCCESS;
  }

  if (optind+2!=argc) {
    WRITE_MSG(2, "Invalid <src> <dst> pair specified\n");
    return EXIT_FAILURE;
  }

  if (mnt.data) {
    mnt.data = strdup(mnt.data);
    if (!transformOptionList(&mnt, 0)) {
      WRITE_MSG(2, "Invalid options specified\n");
      return EXIT_FAILURE;
    }
  }
    
  mnt.src  = argv[optind++];
  mnt.dst  = argv[optind++];

  if (!mountSingle(&mnt, &opt)) return EXIT_FAILURE;
    
  return EXIT_SUCCESS;
}
