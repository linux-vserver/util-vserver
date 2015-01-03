/*	--*- c -*--
 * Copyright (C) 2015 Enrico Scholz <enrico.scholz@ensc.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _BSD_SOURCE	1
#define _DEFAULT_SOURCE	1
#define _ATFILE_SOURCE	1

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <sysexits.h>
#include <unistd.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/mman.h>

#include "compat.h"
#include "pathconfig.h"

#include "../lib_internal/util.h"

#define ENSC_WRAPPERS_STRING 1
#define ENSC_WRAPPERS_STDLIB 1
#include "../ensc_wrappers/wrappers.h"

int wrapper_exit_code = EX_OSERR;

static char const * const	IGNORE_PATTERN[] = {
	"*.rpmnew", "*.rpmsave", "*.rpmorig", "*.cfsaved*", "*.~*~",
	NULL,
};

static void show_help(void)
{
	WRITE_MSG(1,
		  "Usage: systemd-vserver-generator <normal> <early> <late>\n"
		  "\n"
		  "see http://www.freedesktop.org/wiki/Software/systemd/Generators/"
		  "\n"
		  "Please report bugs to " PACKAGE_BUGREPORT "\n");

	exit(0);
}

static void show_version(void)
{
	WRITE_MSG(1,
		  "systemd-vserver-generator " VERSION " -- generates systemd units"
		  "This program is part of " PACKAGE_STRING "\n\n"
		  "Copyright (C) 2015 Enrico Scholz\n"
		  VERSION_COPYRIGHT_DISCLAIMER);
	exit(0);
}

/* escape string accordingly 'systemd.unit(5)' which means that '/' gets
 * turned into '-' and special chars into '\xXX'; prepend a prefix and reserve
 * extra space e.g. for a suffix */
static char *systemd_escape(char const *data, size_t len,
			    char const *prefix,
			    size_t extra_space)
{
	static char const	HEX_DIGIT[] = "0123456789abcdef";

	/* assume the worst case which means that every char must be
	 * encoded */
	char			*dst =
		Emalloc(len * 4 + strlen(prefix) + extra_space + 1);
	char const		*src = data;
	char			*out;

	out = stpcpy(dst, prefix);

	while (src < data + len) {
		unsigned char	c = *src++;

		if (c == '/') {
			*out++ = '-';
		} else if (c == '-' || c < 32 || !isascii(c)) {
			*out++ = '\\';
			*out++ = 'x';
			*out++ = HEX_DIGIT[(c >> 4) & 0x0f];
			*out++ = HEX_DIGIT[(c >> 0) & 0x0f];
		} else {
			*out++ = c;
		}
	}

	*out = '\0';

	return dst;
}

static int open_or_mkdir(int dirfd, char const *name, int flags)
{
	int		fd;
	bool		first_pass = true;

	/* try to open the directory in the first pass; when this fails,
	 * mkdir() it and try again */
	for (;;) {
		fd = openat(dirfd, name, O_DIRECTORY | flags);

		if (fd >= 0)
			break;
		else if (!first_pass) {
			perror("openat()");
			break;
		}

		first_pass = false;

		if (mkdirat(dirfd, name, 0755) < 0 && errno != EEXIST) {
			perror("mkdirat()");
			break;
		}

		/* we created the directory; it can not be a symlink... */
		flags |= O_NOFOLLOW;
	}

	return fd;
}

static int open_or_mkdir_rec(char const *name, int flags)
{
	int	cur_fd = AT_FDCWD;
	char	*tmp = Estrdup(name);

	for (char *ptr = tmp; ptr && *ptr; ) {
		char	*eod;
		int	new_fd;

		eod = ptr;
		while (*eod == '/')
			++eod;
		eod = strchr(eod, '/');

		if (eod) {
			*eod = '\0';
			do {
				++eod;
			} while (*eod == '/');

			if (!*eod)
				eod = NULL;
		}

		new_fd = open_or_mkdir(cur_fd, ptr,
				       eod ? (O_PATH | O_RDWR) : flags);

		if (cur_fd != AT_FDCWD)
			close(cur_fd);

		cur_fd = new_fd;
		if (cur_fd < 0)
			break;

		ptr = eod;
	}

	if (cur_fd == AT_FDCWD){
		errno = EINVAL;
		cur_fd = -1;
	}

	free(tmp);

	return cur_fd;
}

static int xsymlinkat(char const *target, int fd, char const *name)
{
	unlinkat(fd, name, 0);		/* ignore errors */
	return symlinkat(target, fd, name);
}

struct mark_file_data {
	int		unitdir_fd;
	int		targetdir_fd;
	char const	*service_name;
};

static bool mark_handler(char const *data, size_t len, void *res_)
{
	struct mark_file_data const	*mdata = res_;
	char				*mark;
	int				rc;
	int				markdir_fd;
	bool				res = true;

	if (len == 0)
		/* ignore empty names */
		return true;

	/* we create 'vserver-mark@<mark>.target{,.wants}' names */
	mark = systemd_escape(data, len,
			      "vserver-mark@", sizeof ".target.wants");
	strcat(mark, ".target");

	/* create the 'vserver.target -> vserver-mark@<mark>.target'
	 * dependency */
	rc = xsymlinkat("../vserver-mark@.target", mdata->targetdir_fd, mark);
	if (rc < 0) {
		perror("symlinkat(<mark/target>)");
		res = false;
	}

	strcat(mark, ".wants");
	markdir_fd = open_or_mkdir(mdata->unitdir_fd, mark, O_RDWR | O_PATH);
	if (markdir_fd < 0) {
		res = false;
	} else {
		/* create the 'vserver-mark@<mark>.target ->
		 * 'vserver@<vserver>.service' dependency */
		rc = xsymlinkat("../vserver@.service", markdir_fd,
				mdata->service_name);
		if (rc < 0) {
			perror("symlinkat(<mark/service>)");
			res = false;
		}

		close(markdir_fd);
	}

	free(mark);

	return res;
}

struct depends_file_data {
	int		unitdir_fd;
	int		depdir_fd;
	char const	*service_name;
};

static bool depends_handler(char const *data, size_t len, void *res_)
{
	struct depends_file_data	*ddata = res_;
	char				*dep;
	int				rc;
	bool				res = true;

	if (len == 0)
		/* ignore empty lines */
		return true;

	if (ddata->depdir_fd == -1)
		/* do not close it later; it can be reused */
		ddata->depdir_fd = open_or_mkdir(ddata->unitdir_fd,
						 ddata->service_name,
						 O_RDWR | O_PATH);

	if (ddata->depdir_fd < 0)
		return false;

	dep = systemd_escape(data, len, "vserver@", sizeof ".service");
	strcat(dep, ".service");

	rc = xsymlinkat("../vserver@.service", ddata->depdir_fd, dep);
	if (rc < 0) {
		perror("symlinkat(<dep>)");
		res = false;
	}

	free(dep);

	return res;
}

static bool read_file(int dir_fd, char const *name,
		      bool (*line_handler)(char const *line, size_t len,
					   void *data),
		      void *data)
{
	struct stat	st;
	int		fd;
	char const	*mem;
	size_t		len;
	bool		res = true;

	if (fstatat(dir_fd, name, &st, 0) < 0)
		/* skip non existing files */
		return true;

	/* ignore special files; mmap() below works only on regular files */
	if (!S_ISREG(st.st_mode)) {
		WRITE_MSG(2, "read_file() called on non regular file\n");
		return false;
	}

	fd = openat(dir_fd, name, O_RDONLY);
	if (fd < 0) {
		perror("openat()");
		return false;
	}

	len = st.st_size;

	/* mmap() the configuration file */
	mem = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap()");
		close(fd);
		return false;
	}

	close(fd);

	/* iterate over the lines of the configuration file */
	for (char const *ptr = mem; ptr < mem + len && res;) {
		char const	*eol;

		/* skip leading whitespaces (and newlines) */
		while (ptr < mem + len && isspace(*ptr))
			++ptr;

		/* is there a newline? if not, place eol marker to end of
		 * buffer */
		eol = memchr(ptr, '\n', mem + len - ptr);
		if (!eol)
			eol = mem + len;

		/* skip comment lines */
		if (*ptr != '#') {
			char const	*trim = eol;

			/* skip trailing whitespace (including the
			 * newlines) */
			while (trim > ptr && isspace(trim[-1]))
				--trim;

			if (!line_handler(ptr, trim - ptr, data))
				res = false;
		}

		/* we are now at the newline */
		ptr = eol;
	}

	munmap((void *)mem, len);

	return res;
}

static bool generate_units_for_vserver(int unitdir_fd, int vdir_fd,
				       int targetdir_fd,
				       char const *name)
{
	struct stat			st;
	struct mark_file_data		mark_data = {
		.unitdir_fd	= unitdir_fd,
		.targetdir_fd	= targetdir_fd,
	};
	struct depends_file_data	depends_data = {
		.unitdir_fd	= unitdir_fd,
		.depdir_fd	= -1,
	};
	bool				res = true;
	char				*service_name;

	/* check whether name matches some common backup file pattern */
	for (char const * const *p = IGNORE_PATTERN; *p; ++p) {
		if (fnmatch(*p, name, 0) == 0)
			return true;
	}

	if (fstatat(vdir_fd, "disabled", &st, 0) == 0)
		/* vserver is disabled; skip it */
		return true;

	if (fstatat(vdir_fd, "vdir", &st, AT_SYMLINK_NOFOLLOW) < 0 ||
	    !(S_ISDIR(st.st_mode) || S_ISLNK(st.st_mode)))
		/* 'vdir' does not exist or is invalid; skip dir */
		return true;

	/* create the basic 'vserver@<name>.service' name */
	service_name = systemd_escape(name, strlen(name),
				      "vserver@", sizeof ".service.requires");
	strcat(service_name, ".service");

	/* evaluate the 'mark' file and create deps as needed */
	mark_data.service_name = service_name;
	if (!read_file(vdir_fd, "apps/init/mark", mark_handler, &mark_data))
		res = false;

	/* create the 'vserver@<name>.service.requires' name */
	strcat(service_name, ".requires");
	depends_data.service_name = service_name;

	/* read the depends file */
	if (!read_file(vdir_fd, "apps/init/depends",
		       depends_handler, &depends_data))
		res = false;

	/* the depends_handler() can mkdir and open a directory which is
	 * reused for subsequent calls of this handler; close it finally */
	if (depends_data.depdir_fd >= 0)
		close(depends_data.depdir_fd);

	free(service_name);

	return res;
}

static bool generate_units(int unitdir_fd, DIR *vcfg_dir)
{
	int		vcfg_fd = dirfd(vcfg_dir);
	int		targetdir_fd;
	bool		rc = true;

	/* the 'vserver.target' might be needed by rules evaluating the
	 * 'marks' file; create it here */
	targetdir_fd = open_or_mkdir(unitdir_fd, "vserver.target.wants",
				     O_RDWR | O_PATH);
	if (!targetdir_fd)
		return false;

	for (;;) {
		struct dirent	*ent;
		int		vdir_fd;

		errno = 0;
		ent = readdir(vcfg_dir);

		if (!ent && errno) {
			perror("readdir()");
			return false;
		}

		if (!ent)
			break;

		if (ent->d_name[0] == '.')
			/* ignore hidden files (inclusive the special '.' and
			 * '..' directories) */
			continue;

		/* readdir() does not give information about the file type;
		 * determine it manually */
		if (ent->d_type == DT_UNKNOWN) {
			struct stat	st;

			if (!fstatat(vcfg_fd, ent->d_name, &st,
				     AT_SYMLINK_NOFOLLOW) < 0) {
				perror("fstatat()");
				continue;
			}

			/* we are interested in directories only; do not
			 * bother with setting other DT_* values */
			if (S_ISDIR(st.st_mode))
				ent->d_type = DT_DIR;
		}

		if (ent->d_type != DT_DIR)
			/* we are interested in directories only */
			continue;

		/* open the '/etc/vservers/<name>' directory */
		vdir_fd = openat(vcfg_fd, ent->d_name,
				 O_RDONLY | O_DIRECTORY | O_PATH | O_NOFOLLOW);
		if (vdir_fd < 0) {
			perror("openat()");
			continue;
		}

		if (!generate_units_for_vserver(unitdir_fd, vdir_fd,
						targetdir_fd, ent->d_name))
			rc = false;

		close(vdir_fd);
	}

	close(targetdir_fd);

	return rc;
}

int main(int argc, char *argv[])
{
	int		unitdir_fd = -1;
	DIR		*vcfg_dir = NULL;
	char const	*unitdir;
	int		rc;

	if (argc < 2) {
		WRITE_MSG(2, "missing arguments\n");
		return EX_USAGE;
	}
	if (strcmp(argv[1], "--help") == 0)
		show_help();
	if (strcmp(argv[1], "--version") == 0)
		show_version();

	if (argc < 4) {
		WRITE_MSG(2, "bad number of arguments\n");
		return EX_USAGE;
	}

	/* we use the "normal" startup priority unit directory */
	unitdir = argv[1];

	/* create and open the generator directory (usually
	 * /run/systemd/generators) */
	unitdir_fd = open_or_mkdir_rec(unitdir, O_RDWR | O_PATH);
	if (unitdir_fd < 0) {
		perror("mkdir(<unitdir>)");
		return EX_OSERR;
	}

	/* allow to override the CONFDIR (usually /etc/vservers) e.g. for
	 * testing purposes */
	vcfg_dir = opendir(getenv("X_UTIL_VSERVER_CONFDIR") ?
			   getenv("X_UTIL_VSERVER_CONFDIR") : CONFDIR);
	if (!vcfg_dir) {
		perror("opendir(" CONFDIR ")");
		rc = EX_OSERR;
		goto out;
	}

	/* iterate over the vservers */
	rc = generate_units(unitdir_fd, vcfg_dir) ? 0 : EX_OSERR;

out:
	if (vcfg_dir)
		closedir(vcfg_dir);
	if (unitdir_fd != -1)
		close(unitdir_fd);

	return rc;
}
