// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <ensc@delenn.intern.sigma-chemnitz.de>
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

#include "vserver.h"
#include <string.h>
#include <assert.h>

#define DECL(STR, VAL) { STR, sizeof(STR)-1, VAL }

static struct {
    char const * const		id;
    size_t			len;
    unsigned char		val;
} const FLAGVALUES[] = {
  DECL("lock",     S_CTX_INFO_LOCK),
  DECL("sched",    S_CTX_INFO_SCHED),
  DECL("nproc",    S_CTX_INFO_NPROC),
  DECL("private",  S_CTX_INFO_PRIVATE),
  DECL("fakeinit", S_CTX_INFO_INIT),
  DECL("hideinfo", S_CTX_INFO_HIDEINFO),
  DECL("ulimit",   S_CTX_INFO_ULIMIT),
};

unsigned int
vc_text2flag(char const *str, size_t len)
{
  size_t	i;
  if (len==0) len=strlen(str);

  for (i=0; i<sizeof(FLAGVALUES)/sizeof(FLAGVALUES[0]); ++i)
    if (len==FLAGVALUES[i].len &&
	strncmp(FLAGVALUES[i].id, str, len)==0)
      return FLAGVALUES[i].val;

  return 0;
}

char const *
vc_hiflag2text(unsigned int val)
{
  size_t	i;
  size_t	idx;

  assert(S_CTX_INFO_ULIMIT==64);
  
  for (i=S_CTX_INFO_ULIMIT, idx=6; i>0; i/=2, --idx)
    if (val & i) return FLAGVALUES[idx].id;

  return 0;
}
