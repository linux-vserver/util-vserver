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

#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

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
  else
    return mkdirFAIL;
}

static char *
rstrchr(char *str, char c)
{
  while (*str!=c) --str;
  return str;
}

bool
mkdirRecursive(char const *path)
{
  char			buf[strlen(path)+1];
  char *		ptr = buf + sizeof(buf) - 2;

  if (path[0]!='/')      return false; // only absolute paths

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
