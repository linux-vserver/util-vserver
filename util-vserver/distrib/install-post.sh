#!/bin/sh

# Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
# based on distrib/install-post by Jacques Gelinas
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

# Complete the installation of a vserver
: ${UTIL_VSERVER_VARS:=$(dirname $0)/util-vserver-vars}
test -e "$UTIL_VSERVER_VARS" || {
    echo "Can not find util-vserver installation; aborting..."
    exit 1
}
. "$UTIL_VSERVER_VARS"

USR_LIB_VSERVER=$PKGLIBDIR

vserver_mknod()
{
	mknod $1 $2 $3 $4
	chmod $5 $1
}

if [ $# != 1 ] ; then
	echo install-post.sh vserver
else
	VROOT=$VROOTDIR/$1
	rm -fr $VROOT/dev
	mkdir $VROOT/dev && chmod 755 $VROOT/dev
	mkdir $VROOT/dev/pts
	vserver_mknod $VROOT/dev/null c 1 3 666
	vserver_mknod $VROOT/dev/zero c 1 5 666
	vserver_mknod $VROOT/dev/full c 1 7 666
	vserver_mknod $VROOT/dev/random c 1 8 644
	vserver_mknod $VROOT/dev/urandom c 1 9 644
	vserver_mknod $VROOT/dev/tty c 5 0 666
	vserver_mknod $VROOT/dev/ptmx c 5 2 666
	test -f /etc/vservers/$1.conf || cp $USR_LIB_VSERVER/sample.conf /etc/vservers/$1.conf
	test -f /etc/vservers/$1.sh   || cp $USR_LIB_VSERVER/sample.sh /etc/vservers/$1.sh
	echo NETWORKING=yes >$VROOT/etc/sysconfig/network
	echo HOSTNAME=$1 >>$VROOT/etc/sysconfig/network
	(
		cd $VROOT/etc/rc.d/init.d || cd $VROOT/etc/init.d
		for serv in *
		do
			case $serv in
			*.bak|*~|functions|killall|halt|single)
				;;
			*)
				$USR_LIB_VSERVER/capchroot $VROOTDIR/$1 /sbin/chkconfig --level 2345 $serv off
				;;
			esac
		done
		rm -f $VROOT/etc/rc.d/rc6.d/S*reboot
	)
	if [ ! -f $VROOT/etc/fstab ] ; then
		echo /dev/hdv1	/	ext2	defaults	1	1 >$VROOT/etc/fstab
		echo /dev/hdv1	/	ext2	rw	1	1 >$VROOT/etc/mtab
	fi
	cp -a $USR_LIB_VSERVER/vreboot $VROOT/sbin/.
	ln -sf vreboot $VROOT/sbin/vhalt
	if [ -x /etc/vservers/install-post.sh ]; then
		/etc/vservers/install-post.sh $VROOT
	fi
fi

