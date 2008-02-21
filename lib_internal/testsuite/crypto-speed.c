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
#include <time.h>
#include <locale.h>


static void
do_benchmark(char const *meth_name)
{
	struct {
		size_t		block_size;
		size_t		blocks;
	} const			DATA_SIZES[] = {
		{ 0,  1 },
		{ 0,  1024*1024 },
		{ 16, 1 },
		{ 16, 1024*1024 },
		{ 1024, 16 },
		{ 1024, 16*1024 },
		{ 1024*1024, 16 },
		{ 1024*1024, 100 },
		{ 1024*1024, 1000 }
	};
		
	ensc_hash_method const	*m = ensc_crypto_hash_find(meth_name);
	ensc_hash_context	ctx;
	size_t			d_len = m ? ensc_crypto_hash_get_digestsize(m) : 0;
	char			digest[d_len];
	char *			buf;
	size_t			i;

	assert(m);
	assert(ensc_crypto_hashctx_init(&ctx, m)==0);

	for (i=0; i<sizeof(DATA_SIZES)/sizeof(DATA_SIZES[0]); ++i) {
		size_t		cnt = DATA_SIZES[i].blocks;
		size_t const	bs  = DATA_SIZES[i].block_size;
		struct timespec tm_start, tm_end, delta;
		uint64_t	bps;

		buf = malloc(bs+1);	/* avoid malloc-0 confusions */
		assert(buf);

		memset(buf, 0x11, bs);

		ensc_crypto_hashctx_reset(&ctx);

		/* benchmarked code starts here... */
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tm_start);
		while (cnt--)
			ensc_crypto_hashctx_update(&ctx, buf, bs);

		ensc_crypto_hashctx_get_digest(&ctx, digest, NULL, d_len);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tm_end);
		/* ... and ends here */

		delta.tv_sec = tm_end.tv_sec - tm_start.tv_sec;
		if (tm_end.tv_nsec < tm_start.tv_nsec) {
			--delta.tv_sec;
			tm_end.tv_nsec += 1000000000l;
		}
		delta.tv_nsec = tm_end.tv_nsec - tm_start.tv_nsec;

		if (delta.tv_nsec==0 && delta.tv_sec==0)
			delta.tv_nsec = 1;

		bps = (uint64_t)(DATA_SIZES[i].blocks) * bs * 1000000000;
		bps /= (uint64_t)(delta.tv_sec) * 1000000000 + delta.tv_nsec;

		printf("%6s: %7zu x %-7zu -> %2lu.%09lus, %'15llu bytes/s\n",
		       meth_name, DATA_SIZES[i].blocks, bs,
		       delta.tv_sec, delta.tv_nsec, (unsigned long long)(bps));
	}

	ensc_crypto_hashctx_free(&ctx);
}

int main()
{
	char const * const	METHS[] = {
		"md5", "sha1", "sha256", "sha512", NULL
	};
	char const * const *	meth;
	
	ensc_crypto_init();
	setlocale(LC_NUMERIC, "");	/* needed for the thousands grouping */

	for (meth=METHS+0; *meth; ++meth)
		do_benchmark(*meth);
}
