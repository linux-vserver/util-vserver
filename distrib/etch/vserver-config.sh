#!/bin/sh

REMOVE_LINKS="
bootlogd
checkfs
checkroot
halt
hwclock.sh
ifupdown
klogd
libdevmapper1.02
makedev
module-init-tools
mountall.sh
mountdevsubfs.sh
mountnfs.sh
mountkernfs.sh
mountvirtfs
networking
reboot
setserial
single
stop-bootlogd
stop-bootlogd-single
umountfs
umountnfs.sh
umountroot
urandom
"

aptitude update
LANG=C aptitude install locales

test -x /usr/sbin/locale-gen && /usr/sbin/locale-gen

for link in $REMOVE_LINKS; do
	update-rc.d -f $link remove
done

