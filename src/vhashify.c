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

#define UTIL_VSERVER_UNIFY_MTIME_OPTIONAL

#include "vhashify.h"
#include "util.h"

#include "lib/internal.h"
#include "lib_internal/matchlist.h"
#include "lib_internal/unify.h"
#include "ensc_vector/vector.h"

#include <beecrypt/beecrypt.h>

#include <setjmp.h>
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
#define ENSC_WRAPPERS_IO	1
#include <wrappers.h>


#define HASH_BLOCKSIZE		0x10000000u
#define HASH_MINSIZE		0x10
#define HASH_MAXBITS		256		// we have to take care about
						// max filename-length...

#if HASH_MINSIZE<=0
#  error HASH_MINSIZE must be not '0'
#endif


#define CMD_HELP		0x8000
#define CMD_VERSION		0x8001

#define CMD_DESTINATION		0x1000
#define CMD_INSECURE		0x1001
#define CMD_SLEDGE		0x1002
#define CMD_MANUALLY		0x1003
#define CMD_REFRESH		0x1004
#define CMD_NOMTIME		0x1005

struct option const
CMDLINE_OPTIONS[] = {
  { "help",         no_argument,  	0, CMD_HELP },
  { "version",      no_argument,	0, CMD_VERSION },
  { "destination",  required_argument,	0, CMD_DESTINATION },
  { "insecure",     no_argument,       	0, CMD_INSECURE },
  { "sledgehammer", no_argument,      	0, CMD_SLEDGE },
  { "manually",     no_argument,	0, CMD_MANUALLY },
  { "refresh",      no_argument,        0, CMD_REFRESH },
  { "ignore-mtime", no_argument,        0, CMD_NOMTIME },
  { "dry-run",      no_argument,	0, 'n' },
  { "verbose",      no_argument,	0, 'v' },
  { 0,0,0,0 }
};

  // hash digest grouped by 2 digits + hash-collision counter + 2* '/' + NULL
typedef char			HashPath[HASH_MAXBITS/4 + (HASH_MAXBITS/4/2) +
					 sizeof(unsigned int)*2 + 3];

struct HashDirConfiguration
{
    hashFunction const				*method;
    enum { hshALL=0, hshSTART = 1, hshMIDDLE=2,
	   hshEND = 4, hshINVALID = -1 }	blocks;
    size_t					blocksize;
};

struct WalkdownInfo
{
    PathInfo			state;
    struct MatchList		dst_list;
    struct HashDirConfiguration	hash_conf;
    HashDirCollection		hash_dirs;
    size_t			hash_dirs_max_size;

    hashFunctionContext		hash_context;
};

int				wrapper_exit_code = 1;
struct Arguments const		*global_args;
static struct SkipReason	skip_reason;

struct WalkdownInfo		global_info = {
  .hash_conf = { .method     = 0,
		 .blocks     = hshALL,
		 .blocksize  = 0x10000 }
};

#include "vhashify-init.hc"

int Global_getVerbosity() {
  return global_args->verbosity;
}

int Global_doRenew() {
  return true;
}

int Global_isVserverRunning() {
    // TODO
  return global_args->insecure<2;
}

static void
showHelp(char const *cmd)
{
  WRITE_MSG(1, "Usage:\n  ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " [-nv] [--refresh] <vserver>\n    or\n  ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    " --manually [-nv] [--] <hashdir> <path> <excludelist>\n\n"
 	    "  --manually      ...  hashify generic paths; excludelists must be generated\n"
	    "                       manually\n"
	    "  --refresh       ...  hashify already hashified files also\n"
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
  
  switch (Unify_isIUnlinkable(basename->d)) {
    case unifyUNSUPPORTED	:  skip_reason.r = rsUNSUPPORTED; return false;
    case unifyBUSY		:
	// do an implicit refresh on busy files when there are no active links
      if (st->st_nlink>1 && !global_args->do_refresh) {
	  // TODO: message
	skip_reason.r = rsUNIFIED;
	return false;
      }
      break;
    default			:  break;
  }

  return true;
}

static sigjmp_buf		bus_error_restore;
static volatile sig_atomic_t 	bus_error;

static void
handlerSIGBUS(int UNUSED num)
{
  bus_error = 1;
  siglongjmp(bus_error_restore, 1);
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
    if ((in+254)%(in<=2 ? 1 : 256) == 0 && in>0)
      d_path[out++]='/';
    d_path[out++]  = HEX_DIGIT[digest[in] >>    4];
    d_path[out++]  = HEX_DIGIT[digest[in] &  0x0f];
  }
  d_path[out++] = '\0';
  
  return true;
}

#ifndef ENSC_TESTSUITE
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
    .mtime = (global_args->ignore_mtime ? 0 : st->st_mtime),
  };

#undef SET_ATTR
#undef DECL_ATTR

  
  return hashFunctionContextUpdate(h_ctx, (void *)&tmp, sizeof tmp)!=-1;
}
#else
static bool
addStatHash(hashFunctionContext UNUSED *h_ctx, struct stat const UNUSED * const st)
{
  return true;
}
#endif
  
static bool
calculateHashFromFD(int fd, HashPath d_path, struct stat const * const st)
{
  hashFunctionContext * const	h_ctx    = &global_info.hash_context;
  void const * volatile		buf      = 0;
  loff_t volatile		buf_size = 0;
  bool   volatile		res      = false;


  if (hashFunctionContextReset(h_ctx)==-1 ||
      !addStatHash(h_ctx, st))
    return false;

  bus_error = 0;
  if (sigsetjmp(bus_error_restore,1)==0) {
    loff_t			offset   = 0;
    off_t			size     = st->st_size;

    while (offset < size) {
      buf_size = size-offset;
      if (buf_size>HASH_BLOCKSIZE) buf_size = HASH_BLOCKSIZE;

      if ((buf=mmap(0, buf_size, PROT_READ, MAP_SHARED, fd, offset))==0) {
	perror("mmap(<hash>)");
	goto out;
      }

      offset += buf_size;
      madvise(const_cast(void *)(buf), buf_size, MADV_SEQUENTIAL);	// ignore error...

      if (hashFunctionContextUpdate(h_ctx, buf, buf_size)==-1) goto out;

      munmap(const_cast(void *)(buf), buf_size);
      buf = 0;
    }

    res = convertDigest(d_path);
  }

  out:
  if (buf!=0) munmap(const_cast(void *)(buf), buf_size);
  return res;
}

static bool
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

static enum { mkdirFAIL, mkdirSUCCESS, mkdirSKIP }
mkdirSingle(char const *path, char *end_ptr, int good_err)
{
  *end_ptr = '\0';
  if (mkdir(path, 0700)!=-1 || errno==EEXIST) {
    *end_ptr = '/';
    return mkdirSUCCESS;
  }
  else if (errno==good_err) {
    *end_ptr = '/';
    return mkdirSKIP;
  }
  else {
    int		old_errno = errno;
    WRITE_MSG(2, "mkdir('");
    WRITE_STR(2, path);
    errno = old_errno;
    perror("')");
    return mkdirFAIL;
  }
}

static char *
rstrchr(char *str, char c)
{
  while (*str!=c) --str;
  return str;
}

static bool
mkdirRecursive(char const *path)
{
  if (path[0]!='/')      return false; // only absolute paths

  char			buf[strlen(path)+1];
  char *		ptr = buf + sizeof(buf) - 2;

  strcpy(buf, path);

  while (ptr>buf && (ptr = rstrchr(ptr, '/'))!=0) {
    switch (mkdirSingle(buf, ptr, ENOENT)) {
      case mkdirSUCCESS		:  break;
      case mkdirSKIP		:  --ptr; continue;
      case mkdirFAIL		:  return false;
    }

    break;	// implied by mkdirSUCCESS
  }

  assert(ptr!=0);
  ++ptr;

  while ((ptr=strchr(ptr, '/'))!=0) {
    switch (mkdirSingle(buf, ptr, 0)) {
      case mkdirSKIP		:
      case mkdirFAIL		:  return false;
      case mkdirSUCCESS		:  ++ptr; continue;
    }
  }

  return true;
}

static bool
resolveCollisions(char *result, PathInfo const *root, HashPath d_path,
		  struct stat *st, struct stat *hash_st)
{
  strcpy(result, root->d);	// 'root' ends on '/' already (see initHashList())
  strcat(result, d_path);
  
  char 			*ptr = result + strlen(result);
  unsigned int		idx  = 0;
  char			buf[sizeof(int)*2 + 1];
  size_t		len;

  *ptr                 = '-';
  ptr[sizeof(int)*2+1] = '\0';

  for (;; ++idx) {
    len = utilvserver_fmt_xuint(buf, idx);
    memset(ptr+1, '0', sizeof(int)*2 - len);
    memcpy(ptr+1 + sizeof(int)*2 - len, buf, len);

    if (lstat(result, hash_st)==-1) {
      if (global_args->dry_run && errno!=ENOENT) {
	int		old_errno = errno;
	WRITE_MSG(2, "lstat('");
	WRITE_STR(2, buf);
	errno = old_errno;
	perror("')");
	return false;
      }
    }
    else if (Unify_isUnified(st, hash_st)) {
      skip_reason.r = rsUNIFIED;
      return false;
    }
    else if (!Unify_isUnifyable(st, hash_st))
      continue;		// continue with next number*****
    else
      break;		// ok, we finish here

    if (!global_args->dry_run) {
      *ptr = '\0';
      if (!mkdirRecursive(result))
	return false;
      *ptr = '-';

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
    }

      // HACK: avoid an additional lstat on the resulting hash-file
    hash_st->st_size = 0;
    break;
  }

  return true;
}

static char const *
checkDirEntry(PathInfo const *path, PathInfo const *basename,
	      bool *is_dir,
	      struct stat *st, struct stat *hash_st,
	      char *result_buf)
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
	!((skip_reason.r = rsWRONGDEV,
	   (hash_root_path = HashDirInfo_findDevice(&info->hash_dirs, st->st_dev))!=0) &&
	  (skip_reason.r = rsGENERAL,
	   calculateHash(basename, d_path, st)) &&
	  resolveCollisions(result_buf, hash_root_path, d_path, st, hash_st)))
      return 0;

    return result_buf;
  }

  return 0;
}

static void
printSkipReason()
{
  WRITE_MSG(1, " (");
  switch (skip_reason.r) {
    case rsDOTFILE	:  WRITE_MSG(1, "dotfile"); break;
    case rsEXCL		:  WRITE_MSG(1, "excluded"); break;
    case rsTOOSMALL	:  WRITE_MSG(1, "too small"); break;
    case rsUNSUPPORTED	:  WRITE_MSG(1, "operation not supported"); break;
    case rsFSTAT	:  WRITE_MSG(1, "fstat error"); break;
    case rsSYMLINK	:  WRITE_MSG(1, "symlink"); break;
    case rsUNIFIED	:  WRITE_MSG(1, "already unified"); break;
    case rsSPECIAL	:  WRITE_MSG(1, "non regular file"); break;
    case rsWRONGDEV	:  WRITE_MSG(1, "no matching device"); break;
    case rsGENERAL	:  WRITE_MSG(1, "general error"); break;
    default		:  assert(false); abort();
  }
  WRITE_MSG(1, ")");
}

static bool
doit(char const *src, char const *dst,
     struct stat const *src_st, struct stat const *dst_st,
     PathInfo const *path)
{
  if (global_args->dry_run || Global_getVerbosity()>=2) {
    WRITE_MSG(1, "unifying   '");
    Vwrite(1, path->d, path->l);
    WRITE_MSG(1, "'");
    
    if (Global_getVerbosity()>=4) {
      WRITE_MSG(1, " (to '");
      WRITE_STR(1, dst);
      WRITE_MSG(1, "')");
    }

    WRITE_MSG(1, "\n");
  }

    // abort here in dry-run mode
  if (global_args->dry_run) return true;

  if (dst_st->st_size==0) {
      // file was not unified yet
    
    if (Global_isVserverRunning()) {
      (void)unlink(dst);
      if (Unify_copy (src, src_st, dst) &&
	  // the mixed 'dst' and 'src_st' params are intentionally...
	  Unify_unify(dst, src_st, src, false))
	return true;
    }
    else if (Unify_unify(src, src_st, dst, true))
      return true;

    (void)unlink(dst);	// cleanup in error-case
  }
    // there exists already a reference-file
  else if (Unify_unify(dst, dst_st, src, false))
    return true;

  return false;
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
  struct stat			src_stat = { .st_mode=0 };
  struct stat			hash_stat;
  char				tmpbuf[global_info.hash_dirs_max_size +
				       sizeof(HashPath) + 2];
  
  skip_reason.r = rsDOTFILE;

  if (is_dotfile ||
      (match=checkDirEntry(&path, &tmp_path,
			   &is_dir, &src_stat, &hash_stat,
			   tmpbuf))==0) {

    bool	is_link = !is_dotfile && S_ISLNK(src_stat.st_mode);

    if (Global_getVerbosity()>=1 &&
	(Global_getVerbosity()>=3 || skip_reason.r!=rsUNIFIED) &&
	((!is_dotfile && !is_link) ||
	 (Global_getVerbosity()>=6 && is_dotfile) ||
	 (Global_getVerbosity()>=6 && is_link)) ) {
      WRITE_MSG(1, "  skipping '");
      Vwrite(1, path.d, path.l);
      WRITE_MSG(1, "'");
      if (Global_getVerbosity()>=2) printSkipReason();
      WRITE_MSG(1, "\n");
    }

    return 0;
  }

  if (is_dir) {
    res = visitDir(dirname, &src_stat);
  }
  else if (doit(dirname, match, &src_stat, &hash_stat, &path))
    res = 1;
  else {
      // TODO: message
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
    .dry_run            =  false,
    .do_refresh         =  false,
    .ignore_mtime	=  false,
  };

  Vector_init(&global_info.hash_dirs, sizeof(struct HashDirInfo));

  global_args = &args;
  while (1) {
    int		c = getopt_long(argc, argv, "+nv",
				CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(argv[0]);
      case CMD_VERSION		:  showVersion();
      case CMD_DESTINATION	:  args.hash_dir    = optarg; break;
      case CMD_MANUALLY		:  args.mode        = mdMANUALLY; break;
      case CMD_INSECURE		:  args.insecure    = 1;    break;
      case CMD_SLEDGE		:  args.insecure    = 2;    break;
      case CMD_REFRESH		:  args.do_refresh  = true; break;
      case CMD_NOMTIME		:  args.ignore_mtime = true; break;
      case 'n'			:  args.dry_run     = true; break;
      case 'v'			:  ++args.verbosity; break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
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

  if (hashFunctionContextInit(&global_info.hash_context,
			      global_info.hash_conf.method)==-1) {
    WRITE_MSG(2, "Failed to initialize hash-context\n");
    return EXIT_FAILURE;
  }

  if (Global_getVerbosity()>=1)
    WRITE_MSG(1, "Starting to traverse directories...\n");

  signal(SIGBUS, handlerSIGBUS);
  
  Echdir(global_info.dst_list.root.d);
  visitDir("/", 0);

#ifndef NDEBUG
  MatchList_destroy(&global_info.dst_list);
  freeHashList(&global_info.hash_dirs);
  hashFunctionContextFree(&global_info.hash_context);
#endif

  return EXIT_SUCCESS;
}
