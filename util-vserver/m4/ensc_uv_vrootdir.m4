dnl $Id$

dnl Copyright (C) 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
dnl  
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; version 2 of the License.
dnl  
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl  
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

dnl Usage: ENSC_UV_VROOTDIR[(<vserverdir-variable>)]
dnl        <vserverdir-variable> ... name of variable which will get assigned
dnl                                  the dirname of the vserver-topdir 

AC_DEFUN([ENSC_UV_VROOTDIR],
[
	AC_MSG_CHECKING([which vserver-rootdir is to use])
	AC_ARG_WITH([vrootdir],
		    [AC_HELP_STRING([--with-vrootdir=DIR],
				    [place vservers under DIR (default: /vservers)])],
	            [case "$withval" in
			yes|no)	AC_MSG_ERROR(['$withval' is not a valid value for vrootdir]);;
			*)	ensc_uv_path_vrootdir=$withval;;
		     esac],
		    [ensc_uv_path_vrootdir=/vservers])
	AC_MSG_RESULT([$ensc_uv_path_vrootdir])

	if test x"$1" != x; then
		$1=$ensc_uv_path_vrootdir
		AC_SUBST($1)
	fi
])
