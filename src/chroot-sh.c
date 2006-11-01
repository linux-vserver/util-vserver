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

#include <lib_internal/util.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define ENSC_WRAPPERS_PREFIX	"chroot-sh: "
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_IO	1
#define ENSC_WRAPPERS_FCNTL	1
#include <ensc_wrappers/wrappers.h>

int	wrapper_exit_code = EXIT_FAILURE;

static void
showFD(int fd_in, int fd_out)
{
  for (;;) {
    char		buf[4096];
    char const *	ptr=buf;
    ssize_t		len;

    len = Eread(fd_in, buf, sizeof(buf));
    if (len<=0) break;

    EwriteAll(fd_out, ptr, len);
  }
}

static int
redirectFileInternal(int argc, char *argv[],
		     int mode, bool is_input,
		     char const *operation)
{
  int		fd;

  if (argc<2) {
    WRITE_MSG(2, "Not enough parameters for '");
    WRITE_STR(2, operation);
    WRITE_MSG(2, "' operation; use '--help' for more information\n");
    return wrapper_exit_code;
  }

  fd = EopenD(argv[1], mode, 0644);
  if (is_input) showFD(fd,  1);
  else          showFD( 0, fd);
  Eclose(fd);

  return EXIT_SUCCESS;
}

static mode_t
testInternal(int argc, char *argv[], char const *operation)
{
  struct stat		st;
    
  if (argc<2) {
    WRITE_MSG(2, "Not enough parameters for '");
    WRITE_STR(2, operation);
    WRITE_MSG(2, "' operation; use '--help' for more information\n");
    return wrapper_exit_code;
  }

  if (stat(argv[1], &st)==-1) return -1;
  else                        return st.st_mode;
}

static int
execCat(int argc, char *argv[])
{
  return redirectFileInternal(argc, argv,
			      O_RDONLY|O_NOCTTY, true,
			      "cat");
}

static int
execAppend(int argc, char *argv[])
{
  return redirectFileInternal(argc, argv,
			      O_WRONLY|O_CREAT|O_APPEND, false,
			      "append");
}

static int
execTruncate(int argc, char *argv[])
{
  return redirectFileInternal(argc, argv,
			      O_WRONLY|O_CREAT|O_TRUNC, false,
			      "truncate");
}

static int
execRm(int argc, char *argv[])
{
  int		i   = 1;
  int		res = EXIT_SUCCESS;
  
  if (argc<2) {
    WRITE_MSG(2, "No files specified for 'rm' operation; try '--help' for more information\n");
    return wrapper_exit_code;
  }

  for (;i<argc; ++i) {
    if (unlink(argv[i])==-1) {
      PERROR_Q(ENSC_WRAPPERS_PREFIX "unlink", argv[i]);
      res = EXIT_FAILURE;
    }
  }

  return res;
}

static int
execTestFile(int argc, char *argv[])
{
  int		res = testInternal(argc, argv, "testfile");
  
  return res!=-1 && S_ISREG(res) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int
execMkdir(int argc, char *argv[])
{
  int		i   = 1;
  int		res = EXIT_SUCCESS;
  
  if (argc<2) {
    WRITE_MSG(2, "No files specified for 'mkdir' operation; try '--help' for more information\n");
    return wrapper_exit_code;
  }

  for (;i<argc; ++i) {
    if (mkdir(argv[i], 0755)==-1) {
      PERROR_Q(ENSC_WRAPPERS_PREFIX "mkdir", argv[i]);
      res = EXIT_FAILURE;
    }
  }

  return res;
}

static int
execChmod(int argc, char *argv[])
{
  int		i   = 2;
  int		res = EXIT_SUCCESS;
  unsigned long mode;
  
  if (argc<3) {
    WRITE_MSG(2, "No files specified for 'chmod' operation; try '--help' for more information\n");
    return wrapper_exit_code;
  }

  if (!isNumberUnsigned(argv[1], &mode, 1)) {
    WRITE_MSG(2, "Invalid mode: '");
    WRITE_STR(2, argv[1]);
    return EXIT_FAILURE;
  }

  for (;i<argc; ++i) {
    if (chmod(argv[i], mode)==-1) {
      PERROR_Q(ENSC_WRAPPERS_PREFIX "chmod", argv[i]);
      res = EXIT_FAILURE;
    }
  }

  return res;
}

static struct Command {
    char const		*cmd;
    int			(*handler)(int argc, char *argv[]);
} const		COMMANDS[] = {
  { "cat",      execCat },
  { "append",   execAppend },
  { "truncate", execTruncate },
  { "testfile", execTestFile },
  { "rm",       execRm },
  { "mkdir",    execMkdir },
  { "chmod",    execChmod },
  { 0,0 }
};

static void
showHelp()
{
  WRITE_MSG(1,
	    "Usage: chroot-sh "
	    " [--] <cmd> <args>*\n\n"
	    "This program chroots into the current directory and executes the specified\n"
	    "commands there. This means that all used paths are relative to the current\n"
	    "directory, and symlinks can point to files under the current path only.\n"
	    "\n"
	    "The supported commands are:\n"
	    "  cat <file>      ...  gives out <file> to stdout\n"
	    "  append <file>   ...  appends stdin to <file> which is created when needed\n"
	    "  truncate <file> ...  clear <file> and fill it with stdin; the <file> is\n"
	    "                       created when needed\n"
	    "  rm <file>+      ...  unlink the given files\n"
	    "  mkdir <file>+   ...  create the given directories\n"
	    "  chmod <mode> <file>+\n"
	    "                  ...  change access permissions of files\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "chroot-sh " VERSION " -- execute commands within a chroot\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2005 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}


int main(int argc, char *argv[])
{
  struct Command const	*cmd;
  int			idx = 1;
  
  if (argc>=2) {
    if (strcmp(argv[idx], "--help")   ==0) showHelp();
    if (strcmp(argv[idx], "--version")==0) showVersion();
    if (strcmp(argv[idx], "--")==0)        ++idx;
  }

  if (argc<idx+1) {
    WRITE_MSG(2, "No command specified; try '--help' for more information\n");
    return wrapper_exit_code;
  }

  Echroot(".");
  Echdir("/");

  for (cmd=COMMANDS+0; cmd->cmd!=0; ++cmd) {
    if (strcmp(cmd->cmd, argv[idx])==0)
      return cmd->handler(argc-idx, argv+idx);
  }

  WRITE_MSG(2, "Invalid command '");
  WRITE_STR(2, argv[idx]);
  WRITE_MSG(2, "'; try '--help' for more information\n");

  return wrapper_exit_code;
}
