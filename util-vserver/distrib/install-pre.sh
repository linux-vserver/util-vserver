#!/bin/sh

# Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
# based on distrib/install-pre by Jacques Gelinas
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

vserver_mknod()
{
	mknod $1 $2 $3 $4
	chmod $5 $1
}

if [ $# != 1 ] ; then
	echo install-pre.sh vserver
else
	mkdir -p /etc/vservers
	mkdir -p /vservers 2>/dev/null
	VROOT=/vservers/$1
	rm -fr $VROOT/dev
	mkdir -p $VROOT/dev && chmod 755 $VROOT/dev
	mkdir $VROOT/dev/pts
	vserver_mknod $VROOT/dev/null c 1 3 666
	vserver_mknod $VROOT/dev/zero c 1 5 666
	vserver_mknod $VROOT/dev/full c 1 7 666
	vserver_mknod $VROOT/dev/random c 1 8 644
	vserver_mknod $VROOT/dev/urandom c 1 9 644
	vserver_mknod $VROOT/dev/tty c 5 0 666
	vserver_mknod $VROOT/dev/ptmx c 5 2 666
	# We fake this device to help some package managers
	touch $VROOT/dev/hdv1
fi

