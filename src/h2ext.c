// $Id$

// Copyright (C) 2007 Daniel Hokka Zakrisson <daniel@hozac.com>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>

#include "util.h"
#include "lib/internal.h"
#include "pathconfig.h"

#define ENSC_WRAPPERS_PREFIX "h2ext: "
#define ENSC_WRAPPERS_UNISTD 1
#define ENSC_WRAPPERS_FCNTL  1
#define ENSC_WRAPPERS_STAT   1
#include <wrappers.h>

#define MAX_PEEK_SIZE 4096
#define MIN(a,b)  (((a) > (b)) ? (b) : (a))
#define STRINGIFY_(x)	#x
#define STRINGIFY(x)	STRINGIFY_(x)

struct file_format {
  /* where the value would be in the file */
  long		offset;
  /* type of match */
  enum {
    FFT_STRING = 1,
    FFT_SHORT,
    FFT_LONG,
    FFT_LE = 0x4000,
    FFT_BE = 0x8000,
  }		type;
  /* the value */
  union {
    char *	st;
    uint16_t	sh;
    uint32_t	lo;
  }		value;
  /* length of the value */
  size_t	len;
  /* program to use for extraction */
  char *	extractor;
  /* should we try to process the contents as well? */
  int		peek_inside;

  struct file_format *next;
};
typedef struct file_format file_format_t;

int wrapper_exit_code = 255;

#define CMD_HELP		0x4001
#define CMD_VERSION		0x4002

struct option const
CMDLINE_OPTIONS[] = {
  { "help",       no_argument,       0, CMD_HELP },
  { "version",    no_argument,       0, CMD_VERSION },
  { "desc",       required_argument, 0, 'd' },
  { "silent",     no_argument,       0, 'q' },
  { 0,0,0,0 },
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " -d <descriptions file> <file1> [<file2>...]\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "h2ext " VERSION " -- determines how to extract a file\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2007 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

static file_format_t *
find_format(file_format_t *head, char *data)
{
  file_format_t *i;

  for (i = head; i; i = i->next) {
    switch (i->type & ~(FFT_LE|FFT_BE)) {
    case FFT_STRING:
      if (memcmp(i->value.st, data + i->offset, i->len) == 0)
	goto found;
      break;
    case FFT_SHORT:
      if (i->value.sh == *((__typeof__(i->value.sh) *)data + i->offset))
	goto found;
      break;
    case FFT_LONG:
      if (i->value.lo == *((__typeof__(i->value.lo) *)data + i->offset))
	goto found;
      break;
    }
  }
found:
  return i;
}

static int
process_file(file_format_t *head, const char *file, file_format_t *ret[2])
{
  int fd;
  void *mapping;
  struct stat st;

  fd = EopenD(file, O_RDONLY, 0);
  Efstat(fd, &st);
  mapping = mmap(NULL, MIN(st.st_size, MAX_PEEK_SIZE), PROT_READ, MAP_SHARED, fd, 0);
  if (!mapping) {
    perror("mmap()");
    Eclose(fd);
    return -1;
  }

  ret[0] = find_format(head, mapping);

  munmap(mapping, MIN(st.st_size, MAX_PEEK_SIZE));

  if (ret[0] && ret[0]->peek_inside) {
    pid_t child;
    int   fds[2];

    Elseek(fd, 0, SEEK_SET);

    Epipe(fds);
    child = Efork();
    if (child == 0) {
      char *argv[3] = { PROG_H2EXT_WORKER, ret[0]->extractor, NULL };
      dup2(fd, 0);
      dup2(fds[1], 1);
      EexecvpD(PROG_H2EXT_WORKER, argv);
    }
    else {
      char *buf = calloc(MAX_PEEK_SIZE, sizeof(char)), *cur, *end;
      ssize_t bytes_read;

      /* read MAX_PEEK_SIZE bytes from the decompressor */
      cur = buf;
      end = buf + MAX_PEEK_SIZE;
      while (cur < end && (bytes_read = Eread(fds[0], cur, end - cur - 1)) > 0)
	cur += bytes_read;

      /* get rid of the child */
      kill(child, SIGTERM);
      wait(NULL);

      ret[1] = find_format(head, buf);
      free(buf);
    }
  }
  else
    ret[1] = NULL;

  Eclose(fd);

  return 0;
}

static inline void
byteswap(void *p, size_t len)
{
  size_t i;
  char *buf = p, tmp;
  for (i = 0; i < (len >> 1); i++) {
    tmp = buf[len - i - 1];
    buf[len - i - 1] = buf[i];
    buf[i] = tmp;
  }
}

static inline ALWAYSINLINE void
WRITE_INT(int fd, int num)
{
  char   buf[sizeof(num)*3+2];
  size_t l;

  l = utilvserver_fmt_long(buf,num);

  Vwrite(fd, buf, l);
}

static int
load_description(const char *file, file_format_t **head)
{
  file_format_t *prev = NULL,
		*i = NULL;
  int		fd,
		line_no = 0;
  char		buf[512],
		*field,
		*end = buf,
		*ptr,
		*eol;
  ssize_t	bytes_read;

  fd = EopenD(file, O_RDONLY, 0);

  *buf = '\0';
  while (1) {
    if ((eol = strchr(buf, '\n')) == NULL && (end - buf) < (sizeof(buf) - 1)) {
      bytes_read = Eread(fd, end, sizeof(buf) - 1 - (end - buf));
      /* EOF, implicit newline */
      if (bytes_read == 0) {
	if (end == buf)
	  break;
	eol = end;
	*(end++) = '\n';
      }
      end += bytes_read;
      *end = '\0';
      continue;
    }
    else if (eol == NULL) {
      WRITE_MSG(2, ENSC_WRAPPERS_PREFIX);
      WRITE_STR(2, file);
      WRITE_MSG(2, ":");
      WRITE_INT(2, line_no);
      WRITE_MSG(2, " is a really long line\n");
      Eclose(fd);
      return -1;
    }
    *eol = '\0';
    line_no++;

    if (*buf == '#' || *buf == '\0')
      goto new_line;
    if (*head == NULL)
      i = *head = calloc(1, sizeof(file_format_t));
    else {
      i->next = calloc(1, sizeof(file_format_t));
      prev = i;
      i = i->next;
    }
    i->next = NULL;

#define get_field()	if (*(ptr+1) == '\0') goto new_line_and_free; \
			for (ptr++; *ptr == '\t' && *ptr != '\0'; ptr++); \
			for (field = ptr; *ptr != '\t' && *ptr != '\0'; ptr++); \
			*ptr = '\0';
    field = ptr = buf;
    while (*ptr != '\t' && *ptr != '\0')
      ptr++;
    *ptr = '\0';
    if (field == ptr)
      goto new_line_and_free;
    i->offset = strtol(field, NULL, 0);

    get_field();
    if (strcmp(field, "string") == 0)
      i->type = FFT_STRING;
    else if (strcmp(field, "short") == 0)
      i->type = FFT_SHORT;
    else if (strcmp(field, "long") == 0)
      i->type = FFT_LONG;
    else if (strcmp(field, "leshort") == 0)
      i->type = FFT_SHORT|FFT_LE;
    else if (strcmp(field, "beshort") == 0)
      i->type = FFT_SHORT|FFT_BE;
    else if (strcmp(field, "lelong") == 0)
      i->type = FFT_LONG|FFT_LE;
    else if (strcmp(field, "belong") == 0)
      i->type = FFT_LONG|FFT_BE;
    else {
      WRITE_MSG(2, ENSC_WRAPPERS_PREFIX);
      WRITE_STR(2, file);
      WRITE_MSG(2, ":");
      WRITE_INT(2, line_no);
      WRITE_MSG(2, " has an unknown type: ");
      WRITE_STR(2, field);
      WRITE_MSG(2, "\n");
      goto new_line_and_free;
    }

    get_field();
    switch (i->type & ~(FFT_BE|FFT_LE)) {
    case FFT_STRING:
      {
      char *c, *tmp;
      i->value.st = tmp = calloc(strlen(field) + 1, sizeof(char));
      for (c = field; *c; c++) {
	if (*c == '\\') {
	  char *endptr;
	  *(tmp++) = (char)strtol(c + 1, &endptr, 8);
	  c = endptr - 1;
	}
	else
	  *(tmp++) = *c;
      }
      *tmp = '\0';
      i->len = tmp - i->value.st;
      }
      break;
    case FFT_SHORT:
      i->len = sizeof(i->value.sh);
      i->value.sh = (__typeof__(i->value.sh))strtol(field, NULL, 0);
#if BYTE_ORDER != BIG_ENDIAN
      if (i->type & FFT_BE)
#elif BYTE_ORDER != LITTLE_ENDIAN
      if (i->type & FFT_LE)
#else
#  error UNKNOWN BYTE ORDER
#endif
	byteswap(&i->value.sh, i->len);
      break;
    case FFT_LONG:
      i->len = sizeof(i->value.lo);
      i->value.lo = (__typeof__(i->value.lo))strtol(field, NULL, 0);
#if BYTE_ORDER != BIG_ENDIAN
      if (i->type & FFT_BE)
#elif BYTE_ORDER != LITTLE_ENDIAN
      if (i->type & FFT_LE)
#else
#  error UNKNOWN BYTE ORDER
#endif
	byteswap(&i->value.lo, i->len);
      break;
    }

    get_field();
    i->extractor = strdup(field);

    get_field();
    i->peek_inside = (int)strtol(field, NULL, 0);

    /* sanity check the entry */
    if (i->offset < 0) {
      WRITE_MSG(2, ENSC_WRAPPERS_PREFIX);
      WRITE_STR(2, file);
      WRITE_MSG(2, ":");
      WRITE_INT(2, line_no);
      WRITE_MSG(2, " has an invalid offset: ");
      WRITE_INT(2, i->offset);
      WRITE_MSG(2, "\n");
      goto new_line_and_free;
    }
    else if ((i->offset + i->len) > MAX_PEEK_SIZE) {
      WRITE_MSG(2, ENSC_WRAPPERS_PREFIX);
      WRITE_STR(2, file);
      WRITE_MSG(2, ":");
      WRITE_INT(2, line_no);
      WRITE_MSG(2, " exceeds maximum offset (" STRINGIFY(MAX_PEEK_SIZE) ")\n");
      goto new_line_and_free;
    }
#undef get_field
    goto new_line;

new_line_and_free:
    free(i);
    if (prev) {
      i = prev;
      free(i->next);
      i->next = NULL;
    }
    else
      *head = i = NULL;
new_line:
    memmove(buf, eol + 1, end - (eol + 1));
    end = buf + (end - (eol + 1));
  }

  Eclose(fd);
  return 0;
}

int main(int argc, char *argv[])
{
  char		**file = NULL,
		*desc = NULL;
  file_format_t	*head = NULL;
  int		quiet = 0;

  while (1) {
    int c = getopt_long(argc, argv, "+d:q", CMDLINE_OPTIONS, 0);
    if (c == -1) break;

    switch (c) {
    case CMD_HELP:	showHelp(1, argv[0], 0);
    case CMD_VERSION:	showVersion();
    case 'd':		desc = optarg; break;
    case 'q':		quiet = 1;     break;
    default:
      WRITE_MSG(2, "Try '");
      WRITE_STR(2, argv[0]);
      WRITE_MSG(2, " --help' for more information.\n");
      return wrapper_exit_code;
    }
  }

  if (desc == NULL) {
    WRITE_MSG(2, "No descriptions supplied, try '");
    WRITE_STR(2, argv[0]);
    WRITE_MSG(2, " --help' for more information.\n");
    return wrapper_exit_code;
  }

  head = NULL;
  if (load_description(desc, &head) == -1)
    return EXIT_FAILURE;

  for (file = argv + optind; *file; file++) {
    file_format_t *formats[2];
    if (!quiet) {
      WRITE_STR(1, *file);
      WRITE_MSG(1, ": ");
    }
    if (!process_file(head, *file, formats) && formats[0]) {
      WRITE_STR(1, formats[0]->extractor);
      if (formats[0]->peek_inside) {
	WRITE_MSG(1, " | ");
	WRITE_STR(1, formats[1] ? formats[1]->extractor : "unknown format");
      }
    }
    else
      WRITE_MSG(1, "unknown format");
    WRITE_MSG(1, "\n");
  }

  return 0;
}
