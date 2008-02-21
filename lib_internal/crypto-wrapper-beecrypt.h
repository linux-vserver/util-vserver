/*	--*- c -*--
 * Copyright (C) 2008 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 and/or 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_BEECRYPT_H
#define H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_BEECRYPT_H

#include <beecrypt/beecrypt.h>
#include <ctype.h>

typedef hashFunction			ensc_hash_method;
typedef hashFunctionContext		ensc_hash_context;

inline static void
ensc_crypto_init(void)
{
}

inline static ensc_hash_method const *
ensc_crypto_hash_get_default(void)
{
	return hashFunctionDefault();
}

inline static ensc_hash_method const *
ensc_crypto_hash_find(char const *id_c)
{
	char			*id = strdupa(id_c);
	char			*ptr = id;
	char const		*name;

	while (*ptr) {
		*ptr = tolower(*ptr);
		++ptr;
	}

	ptr = id;
	while ((ptr=strchr(ptr, '-'))!=NULL)
		memmove(ptr, ptr+1, strlen(ptr));
	
	if (strcmp(id, "md2")==0)
		name = "MD2";
	else if (strcmp(id, "md5")==0)
		name = "MD5";
	else if (strcmp(id, "sha1")==0)
		name = "SHA-1";
	else if (strcasecmp(id, "sha256")==0)
		name = "SHA-256";
#if 0
	/* sha-384 in beecrypt seems to be broken; digestsize is reported as
	 * 64 there although 48 is the correct value */
	else if (strcasecmp(id, "sha384")==0)
		name = "SHA-384";
#endif
	else if (strcasecmp(id, "sha512")==0)
		name = "SHA-512";
	else
		name = NULL;
		
	return hashFunctionFind(name);
}

inline static char const *
ensc_crypto_hash_get_name(ensc_hash_method const *m)
{
	return m->name;
}

inline static size_t
ensc_crypto_hash_get_digestsize(ensc_hash_method const *m)
{
	return m->digestsize;
}



inline static size_t
ensc_crypto_hashctx_get_digestsize(ensc_hash_context const *ctx)
{
	return ensc_crypto_hash_get_digestsize(ctx->algo);
}

inline static int
ensc_crypto_hashctx_get_digest(ensc_hash_context *ctx, void *result,
			       size_t *res_len, size_t UNUSED max_res_len)
{
	int	rc = hashFunctionContextDigest(ctx, result);
	if (res_len)
		*res_len = ctx->algo->digestsize;

	return rc;
}

inline static int
ensc_crypto_hashctx_update(ensc_hash_context *ctx, void const *src, size_t len)
{
	return hashFunctionContextUpdate(ctx, src, len);
}

inline static int
ensc_crypto_hashctx_init(ensc_hash_context *ctx, ensc_hash_method const *m)
{
	return hashFunctionContextInit(ctx, m);
}

inline static int
ensc_crypto_hashctx_reset(ensc_hash_context *ctx)
{
	return hashFunctionContextReset(ctx);
}

inline static void
ensc_crypto_hashctx_free(ensc_hash_context *ctx)
{
	hashFunctionContextFree(ctx);
}

#endif	/* H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_BEECRYPT_H */
