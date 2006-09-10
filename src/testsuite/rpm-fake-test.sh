#! /bin/bash

: ${srcdir=.}
: ${builddir=.}
: ${srctestsuitedir=./src/testsuite}
: ${srcdatadir=$srctestsuitedir/data}
: ${tmptopdir=/var/tmp}

abssrcdir=$(cd $srcdir && pwd)
absbuilddir=$(cd $builddir && pwd)

DEBUG='strace -E'

set -e

tmpdir=$(mktemp -d /var/tmp/rpm-fake-test.XXXXXX)
trap "rm -rf $tmpdir" EXIT

mkdir -p $tmpdir/{etc,bin}
cat <<EOF >$tmpdir/etc/passwd
root:x:0:0:root:/root:/bin/bash
foo:x:500:500:foo:/:/bin/false
bar:x:501:501:bar:/:/bin/false
EOF

cat <<EOF >$tmpdir/etc/group
root:x:0:root
foo:x:500:foo
bar:x:501:bar
EOF

chmod +rx $tmpdir

RPM_FAKE_RESOLVER_UID=1000 \
RPM_FAKE_RESOLVER_GID=1000 \
RPM_FAKE_CTX=-1 \
RPM_FAKE_RESOLVER=$(pwd)/src/rpm-fake-resolver \
RPM_FAKE_CHROOT=$tmpdir \
RPM_FAKE_NAMESPACE_MOUNTS=/proc \
LD_PRELOAD=$(pwd)/src/.libs/rpm-fake.so \
./src/testsuite/rpm-fake-test <$srcdir/src/testsuite/data/rpm-fake-test.inp
