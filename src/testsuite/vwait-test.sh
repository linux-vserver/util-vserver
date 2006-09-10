#! /bin/bash

: ${srcdir:=.}
: ${builddir:=.}
: ${XID:=1234}

test "$(id -u)" -eq 0 || {
    echo "Skipping vwait-test; it requires root rights" >&2
    exit 0
}


! $builddir/src/vserver-info $XID RUNNING || {
    echo "Skipping vwait-test because context '$XID' is already used" >&2
    exit 0
}

tmpdir=$(mktemp -d /tmp/vwaittest.XXXXXX)
trap "rm -rf $tmpdir" EXIT

: ${VWAIT:=$builddir/src/vwait}
: ${CHCONTEXT:=$builddir/src/chcontext-compat}



############
$CHCONTEXT  --disconnect --silent --xid $XID /bin/bash -c "sleep 2"
status=$($VWAIT --status-fd 1 -- $XID)
rc=$?

case $status  in
    (FINISHED\ *)	;;
    (*)		echo "vwait exited with bad status '$status'/$rc" >&2
		exit 1
esac

wait


############
$CHCONTEXT  --disconnect --silent --xid $XID /bin/bash -c "sleep 5"
status=$($VWAIT --status-fd 1 --timeout 1 -- $XID)
rc=$?

case $status  in
    (TIMEOUT)	;;
    (*)		echo "vwait exited with bad status '$status'/$rc" >&2
		exit 1
esac



############
$CHCONTEXT --disconnect --silent --xid $XID /bin/bash -c "sleep 5"
status=$($VWAIT --terminate --status-fd 1 --timeout 1 -- $XID)
rc=$?

case $status  in
    (KILLED)	;;
    (*)		echo "vwait exited with bad status '$status'/$rc" >&2
		exit 1
esac
