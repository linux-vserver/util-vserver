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

#include <lib_internal/crypto-wrapper.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define ENSC_TESTSUITE
#include "lib_internal/coreassert.h"

#define HASH_BLOCKSIZE		0x10000000u

static bool
convertDigest(char res[], ensc_hash_context * h_ctx)
{
  static char const		HEX_DIGIT[] = "0123456789abcdef";
  size_t			d_size   = ensc_crypto_hashctx_get_digestsize(h_ctx);
    
  unsigned char			digest[d_size];
  size_t			out = 0;

  if (ensc_crypto_hashctx_get_digest(h_ctx, digest, NULL, d_size)==-1)
    return false;
  
  for (size_t in=0; in<d_size; ++in) {
    res[out++]  = HEX_DIGIT[digest[in] >>    4];
    res[out++]  = HEX_DIGIT[digest[in] &  0x0f];
  }
  res[out++] = '\0';
  
  return true;
}

int main(int UNUSED argc, char *argv[])
{
  int				fd = open(argv[1], O_NOFOLLOW|O_NONBLOCK|O_RDONLY|O_NOCTTY);
  ensc_hash_context		hash_context;
  ensc_hash_method const	*method;
  struct stat			st;
  off_t				size;
  loff_t			offset = 0;
  char				digest[2048];

  ensc_crypto_init();
  assert((method = ensc_crypto_hash_find(argv[2]))!=0);
  assert(ensc_crypto_hashctx_init(&hash_context, method)!=-1);

  assert(fstat(fd, &st)!=-1);
  assert(ensc_crypto_hashctx_reset(&hash_context)!=-1);

  size = st.st_size;

  while (offset < size) {
    loff_t volatile		buf_size = size-offset;
    void const *		buf;
    if (buf_size>HASH_BLOCKSIZE) buf_size = HASH_BLOCKSIZE;

    assert((buf=mmap(0, buf_size, PROT_READ, MAP_SHARED, fd, offset))!=0);
    offset += buf_size;
    assert(ensc_crypto_hashctx_update(&hash_context, buf, buf_size)!=-1);
    munmap((void *)(buf), buf_size);
  }
    
  assert(convertDigest(digest, &hash_context));
  
  Vwrite(1, digest, strlen(digest));
  Vwrite(1, "\n", 1);
  
  ensc_crypto_hashctx_free(&hash_context);
  
  return 0;
}
