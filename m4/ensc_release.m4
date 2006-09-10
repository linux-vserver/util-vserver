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

dnl Usage: ENSC_RELEASE(<cppflag-variable>)
dnl        <cppflag-variable> ... name of variable which will get the preprocessor
dnl                               flags and which will be AC_SUBST'ed

AC_DEFUN([ENSC_RELEASE],
[
	AC_MSG_CHECKING([whether to enable release-mode])
	AC_ARG_ENABLE([release],
		      [AC_HELP_STRING([--enable-release],
	                              [enable release mode (default: no)])],
	              [case "$enableval" in
			  yes)  ensc_release_mode=yes;;
                	  no)   ensc_release_mode=no;;
	                  *)    AC_MSG_ERROR(['$enableval' is not a valid value for '--enable-release']);;
	               esac],
        	      [ ensc_release_mode=no ])

	if test x"$ensc_release_mode" = xno; then
		$1=
	else
		$1='-DNDEBUG'
	fi

	AC_SUBST($1)
	AC_MSG_RESULT($ensc_release_mode)
])
