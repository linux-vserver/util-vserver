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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vhashify.h"
#include "util.h"

#include "lib/internal.h"
#include "lib_internal/matchlist.h"
#include "lib_internal/unify.h"
#include "ensc_vector/vector.h"

#include <setjmp.h>
#include <beecrypt/beecrypt.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define ENSC_WRAPPERS_STDLIB	1
#define ENSC_WRAPPERS_UNISTD    1
#define ENSC_WRAPPERS_FCNTL     1
#define ENSC_WRAPPERS_DIRENT    1
#include <wrappers.h>


#define HASH_BLOCKSIZE		0x10000000
#define HASH_MINSIZE		0x10



#define CMD_HELP		0x8000
#define CMD_VERSION		0x8001

#define CMD_DESTINATION		0x1000
#define CMD_INSECURE		0x1001
#define CMD_SLEDGE		0x1002
#define CMD_MANUALLY		0x1003

struct option const
CMDLINE_OPTIONS[] = {
  { "help",         no_argument,  	0, CMD_HELP },
  { "version",      no_argument,	0, CMD_VERSION },
  { "destination",  required_argument,	0, CMD_DESTINATION },
  { "insecure",     no_argument,       	0, CMD_INSECURE },
  { "sledgehammer", no_argument,      	0, CMD_SLEDGE },
  { "manually",     no_argument,	0, CMD_MANUALLY },
  { 0,0,0,0 }
};

  // SHA1, grouped by 4 digits + hash-collision counter + 2* '/' + NULL
typedef char			HashPath[160/4 + (160/4/4) + sizeof(unsigned int)*2 + 3];

struct WalkdownInfo
{
    PathInfo			state;
    struct MatchList		dst_list;
    HashDirCollection		hash_dirs;
    size_t			hash_dirs_max_size;

    hashFunctionContext		hash_context;
};

int				wrapper_exit_code = 1;
struct Arguments const		*global_args;
struct WalkdownInfo		global_info;
static struct SkipReason	skip_reason;

#include "vhashify-init.hc"

int Global_getVerbosity() {
  return global_args->verbosity;
}

int Global_doRenew() {
  return true;
}

static void
showHelp(char const *cmd)
{
  WRITE_MSG(1, "Usage:\n  ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " [-Rnv] <vserver>\n    or\n  ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " --manually [-Rnvx] [--] <hashdir> <path> <excludelist>\n\n"
 	    "  --manually      ...  hashify generic paths; excludelists must be generated\n"
	    "                       manually\n"
	    "  -R              ...  revert operation; dehashify files\n"
	    "  -n              ...  do not modify anything; just show what there will be\n"
	    "                       done (in combination with '-v')\n"
	    "  -v              ...  verbose mode\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vhashify " VERSION " -- hashifies vservers and/or directories\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2005 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

int
HashDirInfo_compareDevice(void const *lhs_v, void const *rhs_v)
{
  struct HashDirInfo const * const	lhs = lhs_v;
  dev_t const * const			rhs = rhs_v;

  assert(lhs!=0 && rhs!=0);
  return lhs->device - *rhs;
}

PathInfo const *
HashDirInfo_findDevice(HashDirCollection const *coll, dev_t dev)
{
  struct HashDirInfo const	*res;

  res = Vector_searchSelfOrg_const(coll, &dev,
				   HashDirInfo_compareDevice, vecSHIFT_ONCE);

  if (res!=0) return &res->path;
  else        return 0;
}

#include "vserver-visitdir.hc"

static bool
checkFstat(PathInfo const * const basename,
	   struct stat * const st)
{
  assert(basename->d[0] != '/');

    // local file does not exist... strange
    // TODO: message
  skip_reason.r = rsFSTAT;
  if (lstat(basename->d, st)==-1) return false;

    // this is a directory and succeeds everytime
  if (S_ISDIR(st->st_mode))
    return true;

    // ignore symlinks
  skip_reason.r = rsSYMLINK;
  if (S_ISLNK(st->st_mode))       return false;

    // ignore special files
  skip_reason.r = rsSPECIAL;
  if (!S_ISREG(st->st_mode) &&
      !S_ISDIR(st->st_mode))      return false;
  
    // ignore small files
  skip_reason.r = rsTOOSMALL;
  if (st->st_size < HASH_MINSIZE) return false;
  
  skip_reason.r = rsUNIFIED;
  if ((!global_args->do_revert && !(st->st_nlink==1 || Unify_isIUnlinkable(basename->d))) ||
      ( global_args->do_revert &&                      Unify_isIUnlinkable(basename->d)))
    return false;

  return true;
}

static jmp_buf			bus_error_restore;
static volatile sig_atomic_t 	bus_error;

static void
handlerSIGBUS(int UNUSED num)
{
  bus_error = 1;
  longjmp(bus_error_restore, 1);
}

static bool
convertDigest(HashPath d_path)
{
  static char const		HEX_DIGIT[] = "0123456789abcdef";
  hashFunctionContext * const	h_ctx    = &global_info.hash_context;
  size_t			d_size   = h_ctx->algo->digestsize;
    
  unsigned char			digest[d_size];
  size_t			out = 0;

  if (hashFunctionContextDigest(h_ctx, digest)==-1)
    return false;
  
  for (size_t in=0;
       out+1<sizeof(HashPath)-(sizeof(unsigned int)*2 + 2) && in<d_size;
       ++in) {
    if (in%2 == 0 && in>0) d_path[out++]='/';
    d_path[out++] = HEX_DIGIT[digest[in] >>    4];
    d_path[out++] = HEX_DIGIT[digest[in] &  0x0f];
  }
  d_path[out++] = '\0';
  
  return true;
}

static bool
addStatHash(hashFunctionContext *h_ctx, struct stat const * const st)
{
#define DECL_ATTR(X)	__typeof__(st->st_##X)	X
#define SET_ATTR(X)	.X = st->st_##X
  
  struct __attribute__((__packed__)) {
    DECL_ATTR(mode);
    DECL_ATTR(uid);
    DECL_ATTR(gid);
    DECL_ATTR(rdev);
    DECL_ATTR(size);
    DECL_ATTR(mtime);
  }		tmp = {
    SET_ATTR(mode),
    SET_ATTR(uid),
    SET_ATTR(gid),
    SET_ATTR(rdev),
    SET_ATTR(size),
    SET_ATTR(mtime)
  };

  return hashFunctionContextUpdate(h_ctx, (void *)&tmp, sizeof tmp)!=-1;
}

static bool
calculateHashFromFD(int fd, HashPath d_path, struct stat const * const st)
{
  hashFunctionContext * const	h_ctx    = &global_info.hash_context;
  bool				res      = false;
  loff_t			offset   = 0;
  void				*buf     = 0;
  off_t				size     = st->st_size;
  loff_t			cur_size = 0;


  if (hashFunctionContextReset(h_ctx)==-1 ||
      !addStatHash(h_ctx, st))
    return false;

  bus_error = 0;
  if (setjmp(bus_error_restore)!=0) goto out;

  while (offset < size) {
    size_t	real_size = size-offset;
    cur_size = real_size;
      //cur_size = (real_size + PAGESIZE-1)/PAGESIZE * PAGESIZE;
    if (cur_size>HASH_BLOCKSIZE) cur_size = HASH_BLOCKSIZE;

    buf     = mmap(0, cur_size, PROT_READ, MAP_SHARED, fd, offset);
    offset += real_size;

    madvise(buf, cur_size, MADV_SEQUENTIAL);	// ignore error...

    if (buf==0) goto out;
    if (hashFunctionContextUpdate(h_ctx, buf, real_size)==-1) goto out;

    munmap(buf, cur_size);
    buf = 0;
  }

  if (!convertDigest(d_path)) goto out;
    
  res = true;

  out:
  if (buf!=0) munmap(buf, cur_size);
  return res;
}

bool
calculateHash(PathInfo const *filename, HashPath d_path, struct stat const * const st)
{
  int		fd  = open(filename->d, O_NOFOLLOW|O_NONBLOCK|O_RDONLY|O_NOCTTY);
  struct stat	fst;
  bool		res = false;

  do {
    if (fd==-1) {
      int	old_errno = errno;
      WRITE_MSG(2, "Failed to open '");
      WRITE_STR(2, filename->d);
      errno = old_errno;
      perror("'");
      break;;
    }
  
    if (fstat(fd, &fst)==-1 ||
	fst.st_dev!=st->st_dev || fst.st_ino!=st->st_ino) {
      WRITE_MSG(2, "An unexpected event occured while stating '");
      WRITE_STR(2, filename->d);
      WRITE_MSG(2, "'.\n");
      break;
    }

    if (!calculateHashFromFD(fd, d_path, st)) {
      WRITE_MSG(2, "Failed to calculate hash for '");
      WRITE_STR(2, filename->d);
      WRITE_MSG(2, "'.\n");
      break;
    }

    res = true;
  } while (false);
  
  if (fd!=-1) close(fd);
  return res;
}

static bool
mkdirRecursive(char const *path)
{
  struct stat		st;

  if (path[0]!='/')       return false; // only absolute paths
  if (lstat(path,&st)!=-1) return true;

  char			buf[strlen(path)+1];
  char *		ptr = buf+1;
  
  strcpy(buf, path);

  while ((ptr = strchr(ptr, '/'))!=0) {
    *ptr = '\0';
    if (mkdir(buf, 0700)==-1 && errno!=EEXIST) {
      int		old_errno = errno;
      WRITE_MSG(2, "mkdir('");
      WRITE_STR(2, buf);
      errno = old_errno;
      perror("')");
      return false;
    }
    *ptr = '/';
    ++ptr;
  }

  return true;
}

static bool
resolveCollisions(char *result, PathInfo const *root, HashPath d_path, struct stat *st)
{
  strcpy(result, root->d);	// 'root' ends on '/' already (see initHashList())
  strcat(result, d_path);
  
  char 			*ptr = result + strlen(result);
  unsigned int		idx  = 0;
  char			buf[sizeof(int)*2 + 1];
  size_t		len;

  *ptr++             = '/';
  *ptr               = '\0';
  ptr[sizeof(int)*2] = '\0';

  if (!mkdirRecursive(result))
    return false;

  for (;; ++idx) {
    len = utilvserver_fmt_xuint(buf, idx);
    memset(ptr, '0', sizeof(int)*2 - len);
    memcpy(ptr + sizeof(int)*2 - len, buf, len);

    struct stat		new_st;
    if (lstat(result, &new_st)==-1) {
      if (errno!=ENOENT) {
	int		old_errno = errno;
	WRITE_MSG(2, "lstat('");
	WRITE_STR(2, buf);
	errno = old_errno;
	perror("')");
	return false;
      }
    }
    else if (!Unify_isUnifyable(st, &new_st))
      continue;		// continue with next number
    else
      break;		// ok, we finish here

    int		fd = open(result, O_NOFOLLOW|O_EXCL|O_CREAT|O_WRONLY, 0200);

    if (fd==-1) {
      int		old_errno = errno;
      WRITE_MSG(2, "open('");
      WRITE_STR(2, buf);
      errno = old_errno;
      perror("')");
      return false;
    }

    close(fd);
    break;
  }

  return true;
}

static char const *
checkDirEntry(PathInfo const *path, PathInfo const *basename,
	      bool *is_dir, struct stat *st, char *result_buf)
{
    //printf("checkDirEntry(%s, %s, %u)\n", path->d, d_path, is_dir);

  struct WalkdownInfo const * const	info       = &global_info;

  // Check if it is in the exclude/include list of the destination vserver and
  // abort when it is not matching an allowed entry
  skip_reason.r      = rsEXCL;
  if (MatchList_compare(&info->dst_list, path->d)!=stINCLUDE) return 0;

  if (checkFstat(basename, st)) {
    PathInfo const	*hash_root_path;
    HashPath 		d_path;
    
    *is_dir = S_ISDIR(st->st_mode);

    if (!*is_dir &&
	!((hash_root_path = HashDirInfo_findDevice(&info->hash_dirs, st->st_dev))!=0 &&
	  calculateHash(basename, d_path, st) &&
	  resolveCollisions(result_buf, hash_root_path, d_path, st)))
      return 0;

    return result_buf;
  }

  return 0;
}

static uint64_t
visitDirEntry(struct dirent const *ent)
{
  uint64_t			res      = 0;
  char const *			dirname  = ent->d_name;
  PathInfo			path     = global_info.state;
  PathInfo			tmp_path = {
    .d = dirname,
    .l = strlen(dirname)
  };
  char				path_buf[ENSC_PI_APPSZ(path, tmp_path)];
  char const			*match = 0;

  
  PathInfo_append(&path, &tmp_path, path_buf);

  bool				is_dotfile    = isDotfile(dirname);
  bool				is_dir;
  struct stat			src_stat;
  char				tmpbuf[global_info.hash_dirs_max_size +
				       sizeof(HashPath) + 2];
  
  skip_reason.r = rsDOTFILE;

  if (is_dotfile ||
      (match=checkDirEntry(&path, &tmp_path,
			   &is_dir, &src_stat, tmpbuf))==0) {

    return 0;
  }

  if (is_dir) {
    res = visitDir(dirname, &src_stat);
  }
  else {
    printf("%s <- %s\n", match, path.d);
    res = 0;
  }

  return res;
    
}

int main(int argc, char *argv[])
{
  struct Arguments	args = {
    .mode               =  mdVSERVER,
    .hash_dir           =  0,
    .verbosity		=  0,
    .insecure           =  0,
  };

  Vector_init(&global_info.hash_dirs, sizeof(struct HashDirInfo));

  if (hashFunctionContextInit(&global_info.hash_context,
			      hashFunctionDefault())==-1)
    return EXIT_FAILURE;


  global_args = &args;
  while (1) {
    int		c = getopt_long(argc, argv, "",
				CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(argv[0]);
      case CMD_VERSION		:  showVersion();
      case CMD_DESTINATION	:  args.hash_dir    = optarg; break;
      case CMD_MANUALLY		:  args.mode        = mdMANUALLY; break;
      case CMD_INSECURE		:  args.insecure    = 1; break;
      case CMD_SLEDGE		:  args.insecure    = 2; break;
      case 'v'			:  ++args.verbosity; break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }

  if (argc==optind) {
    WRITE_MSG(2, "No directory/vserver given\n");
    return EXIT_FAILURE;
  }

  if (args.hash_dir==0 && args.mode==mdMANUALLY) {
    WRITE_MSG(2, "'--manually' requires '--destination'\n");
    return EXIT_FAILURE;
  }

  switch (args.mode) {
    case mdMANUALLY	:  initModeManually(&args, argc-optind, argv+optind); break;
    case mdVSERVER	:  initModeVserver (&args, argc-optind, argv+optind); break;
    default		:  assert(false); return EXIT_FAILURE;
  };

  if (Global_getVerbosity()>=1)
    WRITE_MSG(1, "Starting to traverse directories...\n");

  signal(SIGBUS, handlerSIGBUS);
  
  Echdir(global_info.dst_list.root.d);
  visitDir("/", 0);  
}
