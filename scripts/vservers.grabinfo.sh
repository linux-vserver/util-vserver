#!/bin/sh
# Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
# based on vservers.grabinfo.sh by Jacques Gelinas
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

# Produce some XML statistics about vservers

: ${UTIL_VSERVER_VARS:=/usr/lib/util-vserver/util-vserver-vars}
test -e "$UTIL_VSERVER_VARS" || {
    echo $"Can not find util-vserver installation (the file '$UTIL_VSERVER_VARS' would be expected); aborting..." >&2
    exit 1
}
. "$UTIL_VSERVER_VARS"

cd $__DEFAULT_VSERVERDIR
for vserv in *
do
	if [ -f /etc/vservers/$vserv.conf ] ; then
		. /etc/vservers/$vserv.conf
		echo "<m:vserver name=\"$vserv\" onboot=\"$ONBOOT\" HOSTNAME=\"$S_HOSTNAME\">"
		for ip in $IPROOT
		do
			case $ip in
			*:*)
				echo $ip | tr ':' ' ' | (read a b; echo "  <m:ip num=\"$b\"/>")
				;;
			*)
				echo "  <m:ip num=\"$ip\"/>"
				;;
			esac
		done
		echo "  <status>"
		$_VSERVER $vserv status
		echo "  </status>"
		echo "</m:vserver>"
	fi
done

