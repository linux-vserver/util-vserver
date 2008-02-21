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

#define ENSC_TESTSUITE

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <lib_internal/crypto-wrapper.h>
#include <lib_internal/coreassert.h>

static unsigned int
hex2digit(char c)
{
	if (c>='0' && c<='9')
		return c-'0';
	c &= ~0x20;
	if (c>='A' && c<='F')
		return c-'A' + 10;

	assert(0);
	return 0;
}

static void
convert_digest_ascii2bin(void *dst_v, char const *digest, size_t d_len)
{
	unsigned char	*dst = dst_v;
	
	while (d_len>0) {
		*dst  = hex2digit(*digest++)<<4;
		*dst |= hex2digit(*digest++);

		++dst;
		--d_len;
	}
}

static void
test_digest(char const *name, size_t d_len,
	    void const *buf,  size_t buf_len,
	    char const *digest)
{
	ensc_hash_method const	*m = ensc_crypto_hash_find(name);
	ensc_hash_context	ctx;
	unsigned char		*exp_digest[d_len/8];
	unsigned char		*bin_digest[d_len/8];
	size_t			bin_digest_len;
	size_t			i;

	d_len /= 8;
	convert_digest_ascii2bin(exp_digest, digest, d_len);

	assert(m);
	assert(ensc_crypto_hash_get_digestsize(m)==d_len);

	{
		char const		*tmp_name = ensc_crypto_hash_get_name(m);
		ensc_hash_method const	*tmp_meth = tmp_name ? ensc_crypto_hash_find(tmp_name) : NULL;

		assert(tmp_name!=NULL);
		assert(tmp_meth!=NULL);
		assert(ensc_crypto_hash_get_digestsize(tmp_meth)==d_len);
	}

	ensc_crypto_hashctx_init(&ctx, m);
	assert(ensc_crypto_hashctx_get_digestsize(&ctx)==d_len);

	/* run it multiple times to test for correct reset/init behavior */
	for (i=0; i<3; ++i) {
		assert(ensc_crypto_hashctx_reset(&ctx)==0);
		assert(ensc_crypto_hashctx_update(&ctx, buf, buf_len)==0);

		switch (i) {
		case 0:
		case 2:
			break;

		case 1:
			assert(ensc_crypto_hashctx_update(&ctx, "gremlin", 7)==0);
			break;
		}

		assert(ensc_crypto_hashctx_get_digest(&ctx, bin_digest, &bin_digest_len, d_len)==0);
		assert(bin_digest_len==d_len);

		
		switch (i) {
		case 0:
		case 2:
			assert(memcmp(exp_digest, bin_digest, d_len)==0);
			break;

		case 1:
			assert(memcmp(exp_digest, bin_digest, d_len)!=0);
			break;
		}
	}

	ensc_crypto_hashctx_free(&ctx);
}

int main()
{
	ensc_crypto_init();
	assert(ensc_crypto_hash_get_default()!=NULL);

	/* MD-5 */

	test_digest("md5",  128, "",    0, "d41d8cd98f00b204e9800998ecf8427e");
	test_digest("md-5", 128, "",    0, "d41d8cd98f00b204e9800998ecf8427e");
	test_digest("MD5",  128, "",    0, "d41d8cd98f00b204e9800998ecf8427e");
	test_digest("MD-5", 128, "",    0, "d41d8cd98f00b204e9800998ecf8427e");

	test_digest("md5",  128, "foo", 3, "acbd18db4cc2f85cedef654fccc4a4d8");
	
	/* SHA-1 */
	test_digest("sha1",  160, "",    0, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
	test_digest("sha-1", 160, "",    0, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
	test_digest("SHA1",  160, "",    0, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
	test_digest("SHA-1", 160, "",    0, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
	
	test_digest("sha1",  160, "foo", 3, "0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33");

	/* SHA-256 */
	test_digest("sha256",  256, "",    0, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	test_digest("sha-256", 256, "",    0, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	test_digest("SHA256",  256, "",    0, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	test_digest("SHA-256", 256, "",    0, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

	test_digest("sha256",  256, "foo", 3, "2c26b46b68ffc68ff99b453c1d30413413422d706483bfa0f98a5e886266e7ae");

#if ENSC_CRYPTO_API != ENSC_CRYPTO_API_BEECRYPT	 /* see comments in crypto-wrapper-beecrypt.h */
	/* SHA-384 */
	test_digest("sha384",  384, "",    0, "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
	test_digest("sha-384", 384, "",    0, "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
	test_digest("SHA384",  384, "",    0, "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
	test_digest("SHA-384", 384, "",    0, "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");

	test_digest("sha384",  384, "foo", 3, "98c11ffdfdd540676b1a137cb1a22b2a70350c9a44171d6b1180c6be5cbb2ee3f79d532c8a1dd9ef2e8e08e752a3babb");
#endif

	/* SHA-512 */
	test_digest("sha512",  512, "",    0, "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
	test_digest("sha-512", 512, "",    0, "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
	test_digest("SHA512",  512, "",    0, "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
	test_digest("SHA-512", 512, "",    0, "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");

	test_digest("sha512",  512, "foo", 3, "f7fbba6e0636f890e56fbbf3283e524c6fa3204ae298382d624741d0dc6638326e282c41be5e4254d8820772c5518a2c5a8c0c7f7eda19594a7eb539453e1ed7");
}
