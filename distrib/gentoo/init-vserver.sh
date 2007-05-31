#!/bin/bash
#
# Copyright (C) 2006 Benedikt Boehm <hollow@gentoo.org>
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

#
# BIG FAT WARNING:
#
# Do not remove this file if you are using gentoo init style!
# Your vserver will not boot anymore!
#
# You have been warned...

# Force TERM=linux for baselayout-2

RUNLEVEL=1 /sbin/rc sysinit || exit 1
/sbin/rc boot || exit 1
/sbin/rc ${1:-default}
exit 0
