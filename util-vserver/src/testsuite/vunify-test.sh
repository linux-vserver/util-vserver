#! /bin/bash

: ${srcdir=.}
: ${tmptopdir=/var/tmp}

set -e

tmpdir=$(mktemp -d /var/tmp/vunify-test.XXXXXX)
trap "rm -rf $tmpdir" EXIT

function createFiles
{
    local base=$1
    shift

    local idx=0
    local i
    for i; do
	echo $i >$base$idx
	let ++idx
    done
}

function createSet
{
    local base=$1

    createFiles a/$base    a  b  c d
    createFiles b/$base    a  b  c
    createFiles c/$base    XX XX

    ln a/${base}3  b/${base}3
}

pushd $tmpdir &>/dev/null
    mkdir -p {a,b,c}{/etc/sysconfig,/usr/lib,/usr/local/lib/foobar,/var/run}

    createSet etc/CFG
    createSet etc/sysconfig/CFG

    createSet usr/PROG
    createSet usr/lib/PROG

    createSet usr/local/lib/LOCAL
    createSet usr/local/lib/foobar/LOCAL

    createSet var/TEMP
    createSet var/run/TEMP
popd &>/dev/null


$D ./src/vunify -n --manually $tmpdir/a '' $tmpdir/b '' >/dev/null
$D ./src/vunify -n --manually $tmpdir/a '' $tmpdir/c '' >/dev/null
