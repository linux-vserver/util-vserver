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

dnl Usage: ENSC_KERNEL_HEADERS(<var>)
dnl        <var> ... basedir of kernel-headers (without the '/linux');
dnl                  this value will be AC_SUBST'ed

AC_DEFUN([_ENSC_KERNEL_DIR],
[
	AC_CACHE_CHECK([for linux kernel dir], [ensc_cv_path_kerneldir],
        [
AC_ARG_WITH([kerneldir],
	    [AC_HELP_STRING([--with-kerneldir=DIR],
      		            [assume kernelsources in DIR (default: /lib/modules/<current>/build)])],
            [case "$withval" in
		yes|no)	AC_MSG_ERROR(['$withval' is not a valid value for kerneldir]);;
		*)	ensc_cv_path_kerneldir=$withval;;
	     esac],
	    [ensc_cv_path_kerneldir=
	     for i in /lib/modules/$(uname -r)/build /usr/src/linux /usr; do
		test -e $i/include/linux/version.h && { ensc_cv_path_kerneldir=$i; break; }
	     done])
	])

	test "$ensc_cv_path_kerneldir" -a -e "$ensc_cv_path_kerneldir"/include/linux/version.h || {
		AC_MSG_ERROR([Can not find kernelsources])
	}
])

AC_DEFUN([ENSC_KERNEL_HEADERS],
[
	AC_REQUIRE([_ENSC_KERNEL_DIR])

	AC_CACHE_CHECK([for linux kernel headers], [ensc_cv_path_kernelheaders],
	[
		ensc_cv_path_kernelheaders=$ensc_cv_path_kerneldir/include
	])

	$1=$ensc_cv_path_kernelheaders
	AC_SUBST($1)
])
