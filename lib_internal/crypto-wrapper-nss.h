/*	--*- c -*--
 * Copyright (C) 2008 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#ifndef H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_NSS_H
#define H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_NSS_H

#include <sechash.h>
#include <secoid.h>
#include <nss.h>

#include "util-cast.h"

typedef struct SECHashObjectStr		ensc_hash_method;
typedef struct HASHContextStr		*ensc_hash_context;

inline static int
ensc_crypto_init(void)
{
	NSS_NoDB_Init(NULL);
	return 0;
}

inline static ensc_hash_method const *
ensc_crypto_hash_get_default(void)
{
	return HASH_GetHashObject(SEC_OID_SHA1);
}

inline static ensc_hash_method const *
ensc_crypto_hash_find(char const *id_c)
{
	SECOidTag		oid;

#if 1
	char			*id = strdupa(id_c);
	char			*ptr = id;

	while (*ptr) {
		*ptr = tolower(*ptr);
		++ptr;
	}

	ptr = id;
	while ((ptr=strchr(ptr, '-'))!=NULL)
		memmove(ptr, ptr+1, strlen(ptr));
	
	if (strcmp(id, "md2")==0)
		oid = SEC_OID_MD2;
	else if (strcmp(id, "md5")==0)
		oid = SEC_OID_MD5;
	else if (strcmp(id, "sha1")==0)
		oid = SEC_OID_SHA1;
	else if (strcasecmp(id, "sha256")==0)
		oid = SEC_OID_SHA256;
	else if (strcasecmp(id, "sha384")==0)
		oid = SEC_OID_SHA384;
	else if (strcasecmp(id, "sha512")==0)
		oid = SEC_OID_SHA512;
	else
		oid = SEC_OID_UNKNOWN;
		
#else
	struct SECItemStr const	item = {
		.type = ???,
		.data = const_cast(unsigned char *)(static_cast(unsigned char const *)(id)),
		.len  = strlen(id)
	};
	SECOidTag		oid;
	
	oid = SECOID_FindOIDTag(&item);
#endif

	return HASH_GetHashObjectByOidTag(oid);
}

inline static char const *
ensc_crypto_hash_get_name(ensc_hash_method const *m)
{
	char const * const	NAMES[] = {
		[HASH_AlgNULL]   = "null",
		[HASH_AlgMD2]    = "md2",
		[HASH_AlgMD5]    = "md5",
		[HASH_AlgSHA1]   = "sha1",
		[HASH_AlgSHA256] = "sha256",
		[HASH_AlgSHA384] = "sha384",
		[HASH_AlgSHA512] = "sha512",
	};
	size_t		idx = static_cast(size_t)(m->type);
	
	if (idx >= sizeof(NAMES)/sizeof(NAMES[0]))
		return NULL;
	
	return NAMES[idx];
	/* TODO: use SECOID_FindOIDTagDescription()? */
}

inline static size_t
ensc_crypto_hash_get_digestsize(ensc_hash_method const *m)
{
	size_t const		SIZES[] = {
		[HASH_AlgMD2]    = MD2_LENGTH,
		[HASH_AlgMD5]    = MD5_LENGTH,
		[HASH_AlgSHA1]   = SHA1_LENGTH,
		[HASH_AlgSHA256] = SHA256_LENGTH,
		[HASH_AlgSHA384] = SHA384_LENGTH,
		[HASH_AlgSHA512] = SHA512_LENGTH,
	};
	size_t		idx = static_cast(size_t)(m->type);
	
	if (idx >= sizeof(SIZES)/sizeof(SIZES[0]))
		return 0;
	
	return SIZES[idx];
}

inline static size_t
ensc_crypto_hashctx_get_digestsize(ensc_hash_context const *ctx)
{
	return ensc_crypto_hash_get_digestsize((*ctx)->hashobj);
}

inline static int
ensc_crypto_hashctx_get_digest(ensc_hash_context *ctx, void *result,
			       size_t UNUSED *res_len_a, size_t UNUSED max_res_len)
{
	unsigned int	res_len;
	
	HASH_End(*ctx, result, &res_len, max_res_len);
	if (res_len_a)
		*res_len_a = res_len;

	return 0;
}

inline static int
ensc_crypto_hashctx_init(ensc_hash_context *ctx, ensc_hash_method const *m)
{
	*ctx = HASH_Create(m->type);
	return *ctx==NULL ? -1 : 0;
}

inline static int
ensc_crypto_hashctx_update(ensc_hash_context *ctx, void const *src, size_t len)
{
	HASH_Update(*ctx, src, len);
	return 0;
}

inline static int
ensc_crypto_hashctx_reset(ensc_hash_context *ctx)
{
	HASH_Begin(*ctx);
	return 0;
}

inline static void
ensc_crypto_hashctx_free(ensc_hash_context *ctx)
{
	HASH_Destroy(*ctx);
	*ctx = NULL;
}

#endif	/* H_UTIL_VSERVER_LIB_INTERNAL_CRYPTO_WRAPPER_NSS_H */
