#! /bin/bash

# Copyright (C) 2005 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

: ${srcdir=.}
: ${builddir=.}
: ${srctestsuitedir=$srcdir/src/testsuite}
: ${srcdatadir=$srctestsuitedir/data}
: ${tmptopdir=/var/tmp}
: ${hashcalc:=$builddir/src/testsuite/hashcalc}

set -e

tmpdir=$(mktemp -d "$tmptopdir"/rpm-fake-test.XXXXXX)
trap "rm -rf $tmpdir" EXIT

## Usage: createRandFile <name> <size>
function createRandFile
{
    dd if=/dev/urandom of=$tmpdir/$1-$2 bs=$2 count=1 &>/dev/null
}

pg=$(getconf PAGESIZE)

for i in 2 4 8 15 16 23 42 32 64 68 $pg $[ pg+42 ] $[ pg*2 ] \
	 $[ pg*2-23 ] $[ pg*23+42 ]; do
    createRandFile rand $[ i - 1 ]
    createRandFile rand $i
    createRandFile rand $[ i + 1 ]
done

: > $tmpdir/rand-0

test x"$ensc_use_expensive_tests" != xyes || {
    dd if=/dev/urandom of=$tmpdir/rand-LARGE1 bs=$[ pg-1 ]        count=1 seek=124123
    dd if=/dev/urandom of=$tmpdir/rand-LARGE2 bs=$[ 1024*1024-1 ] count=1 seek=5003
    #dd if=/dev/urandom of=$tmpdir/rand-LARGE3 bs=$[ pg-1 ] count=1 seek=12412373
} &>/dev/null

for i in $tmpdir/rand-*; do
    for m in md5 sha1 sha256 sha512; do
	sum_0=$($hashcalc "$i" "$m" | tr -d / )
	sum_1=$(${m}sum   "$i" | awk '{ print $1}' )

	# compare only the first 80 chars as vhashify will cut digest to MAXPATHLEN
	test x"${sum_0::80}" = x"${sum_1::80}" || {
	  echo "$m mismatch at $(basename $i): '$sum_0' vs. '$sum_1'"
	  exit 1
	}
    done
done

true
