#!/bin/sh

# Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
# based on vunify by Jacques Gelinas
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

# This scripts is used to unify the disk space used by vservers
# It takes various RPM packages and hard link them together so all
# vservers are sharing the same exact copy of the files.
# After doing so, it set them immutable, so the vserver can't change them

# This has the following advantages:
#	-You save disk space. If you have 100 vservers, each using 500 megs
#	 (common linux server installation), you can unify 90% of that
#	-Memory usage. Since the exact same binary are loaded, including
#	 the same shared object, you save some memory and this can increase
#	 performance, especially the memory cache usage.
#
# On the down side, you are loosing some flexibility. The vserver
# administrators can't upgrade package as they see fit, since the
# files are immutable. On the other end, just unifying glibc is probably
# a win.
: ${UTIL_VSERVER_VARS:=$(dirname $0)/util-vserver-vars}
test -e "$UTIL_VSERVER_VARS" || {
    echo "Can not find util-vserver installation; aborting..."
    exit 1
}
. "$UTIL_VSERVER_VARS"

if [ $# = 0 ] ; then
	echo vunify [ --undo ] ref-vserver vservers -- packages
else
	undo=0
	if [ "$1" == "--undo" ] ; then
		undo=1
		shift
	fi
	ref=$1
	shift
	servers=
	while [ "$1" != "" -a "$1" != "--" ]
	do
		servers="$servers $1"
		shift
	done
	if [ "$servers" = "" ] ; then
		echo No vserver specified >&2
		exit 1
	elif [ "$1" != "--" ] ; then
		echo Missing -- marker >&2
		exit 1
	else
		shift
		if [ $# = 0 ] ; then
			echo No package specified >&2
			exit 1
		else
			if [ ! -d $VROOTDIR/$ref/. ] ; then
				echo No vserver $ref >&2
				exit 1
			else
				#echo ref=$ref
				#echo servers=$servers
				#echo packages=$*
				tmpfile=/var/run/vunifi.$$
				rm -f $tmpfile
				echo Extracting list of file to unify in $tmpfile
				for pkg in $*
				do
					$VROOTDIR/$ref/bin/rpm --root $VROOTDIR/$ref -ql --dump $pkg | \
					while read path size mtime md5 \
						mode owner group isconfig isdoc rdev symlink
					do
						if [ "$isconfig" = 0 ] ; then
							echo $path >>$tmpfile
						fi
					done
				done
				for serv in $servers
				do
					if [ "$undo" = 0 ] ; then
						echo Unifying server $serv
						cat $tmpfile | while read file
						do
							if [ ! -d $VROOTDIR/$ref/$file -a ! -L $VROOTDIR/$ref/$file ] ; then
								ln -f $VROOTDIR/$ref/$file $VROOTDIR/$serv/$file
							fi
						done
						cat $tmpfile | while read file
						do
							chattr +i $VROOTDIR/$ref/$file
						done
					else
						echo Differencing server $serv
						cat $tmpfile | while read file
						do
							chattr -i $VROOTDIR/$ref/$file
							if [ ! -d $VROOTDIR/$ref/$file ] ; then
								rm -f $VROOTDIR/$serv/$file
								cp -a $VROOTDIR/$ref/$file $VROOTDIR/$serv/$file
							fi
						done
					fi
				done
				rm -f $tmpfile 
			fi
		fi
	fi
fi

