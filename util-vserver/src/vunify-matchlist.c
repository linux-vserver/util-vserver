// $Id$    --*- c -*--

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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vunify-matchlist.h"
#include "wrappers.h"

#include <fnmatch.h>
#include <assert.h>

bool
MatchList_compare(struct MatchList const *list, char const *path)
{
  bool					res = true;
  struct MatchItem const *		ptr = list->data;
  struct MatchItem const * const	end_ptr = list->data + list->count;
  
  //write(1, path, strlen(path));
  //write(1, "\n", 1);
  for (; ptr<end_ptr; ++ptr) {
    if ((ptr->cmp==0 && strcmp(ptr->name, path)==0) ||
	(ptr->cmp!=0 && (ptr->cmp)(ptr->name, path)==0)) {
      switch (ptr->type) {
	case stINCLUDE	:  res = true;  break;
	case stEXCLUDE	:  res = false; break;
	default		:  abort();
      }
      break;
    }
  }

  return res;
}

void
MatchList_init(struct MatchList *list, char const *root, size_t count)
{
  list->skip_depth = 0;
  list->root.d     = root;
  list->root.l     = strlen(root);
  list->data       = Emalloc(sizeof(struct MatchItem) * count);
  list->count      = count;
  list->buf        = 0;
  list->buf_count  = 0;

  String_init(&list->id);
}

static int
fnmatchWrap(char const *a, char const *b)
{
  return fnmatch(a, b, 0);
}


static MatchItemCompareFunc
determineCompareFunc(char const UNUSED *fname)
{
  return fnmatchWrap;
}

void
MatchList_appendFiles(struct MatchList *list, size_t idx,
		      char **files, size_t count,
		      bool auto_type)
{
  struct MatchItem	*ptr = list->data + idx;
  size_t		i;
  
  assert(idx+count <= list->count);

  if (auto_type) {
    for (i=0; i<count; ++i) {
      char	*file = files[i];
      switch (file[0]) {
	case '+'	:  ptr->type = stINCLUDE; ++file; break;
	case '-'	:  ++file; /*@fallthrough@*/
	default		:  ptr->type = stEXCLUDE; break;
      }
      ptr->cmp  = determineCompareFunc(file);
      ptr->name = file;
      ++ptr;
    }
  }
  else {
    for (i=0; i<count; ++i) {
      ptr->type = stEXCLUDE;
      ptr->name = files[i];
      ptr->cmp  = 0;
      ++ptr;
    }
  }
}


void
PathInfo_append(PathInfo       * restrict lhs,
		PathInfo const * restrict rhs,
		char *buf)
{
  char *		ptr = buf;
  char const *		rhs_ptr = rhs->d;
  size_t		rhs_len = rhs->l;

  while (lhs->l>1 && lhs->d[lhs->l-1]=='/') --lhs->l;

  if (lhs->l>0) {
    while (rhs->l>0 && *rhs_ptr=='/') {
      ++rhs_ptr;
      --rhs_len;
    }
    
    ptr = Xmemcpy(ptr, lhs->d, lhs->l);
    if (ptr[-1]!='/')
      ptr = Xmemcpy(ptr, "/", 1);
  }
//  else if (*rhs_ptr!='/')
//    ptr = Xmemcpy(ptr, "/", 1);

  ptr = Xmemcpy(ptr, rhs_ptr, rhs_len+1);

  lhs->d = buf;
  lhs->l = ptr-buf-1;
}

void
String_init(String *str)
{
  str->d = 0;
  str->l = 0;
}

#ifdef ENSC_TESTSUITE
#define CHECK(LHS,RHS, EXP)				\
  do {							\
    PathInfo	lhs  = { LHS, sizeof(LHS)-1 };		\
    PathInfo	rhs  = { RHS, sizeof(RHS)-1 };		\
    char	*buf = malloc(ENSC_PI_APPSZ(lhs,rhs));	\
    assert(ENSC_PI_APPSZ(lhs,rhs)>=sizeof(EXP));	\
    PathInfo_append(&lhs, &rhs, buf);			\
    assert(memcmp(lhs.d, EXP, sizeof(EXP))==0);		\
    assert(lhs.l == sizeof(EXP)-1);			\
    free(buf);						\
  } while (0)


void
PathInfo_test()
{
  CHECK("/var",  "/tmp", "/var/tmp");
  CHECK("/var",   "tmp", "/var/tmp");
  CHECK("/var/", "/tmp", "/var/tmp");
  CHECK("/var/",  "tmp", "/var/tmp");
  
  CHECK("/",  "tmp",   "/tmp");
  CHECK("/", "/tmp",   "/tmp");
  
  CHECK("", "/tmp",   "/tmp");
  
  CHECK("", "tmp",    "tmp");
  CHECK("", "",       "");
}
#endif
