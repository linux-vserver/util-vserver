dnl $Id$

dnl Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

AC_DEFUN([ENSC_DIETLIBC_NEED_COMPAT],
[
	AC_REQUIRE([ENSC_ENABLE_DIETLIBC])
	AC_CACHE_CHECK([whether dietlibc needs '-lcompat'], [ensc_cv_c_dietlibc_compat],
	[
		AC_LANG_PUSH(C)
		old_CC=$CC
		CC="${DIET:-diet} $CC"
		AC_LINK_IFELSE([
			AC_LANG_PROGRAM([
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>

#define __NR_foo		0
inline static _syscall0(int, foo)
],
			[foo()])],
			[ensc_cv_c_dietlibc_compat=no],
			[ensc_cv_c_dietlibc_compat=yes])
		CC=$old_CC
		AC_LANG_POP
	])

	if test x"$1" != x; then
		AM_CONDITIONAL($1, test x"$ensc_cv_c_dietlibc_compat" = xyes)
	fi
])
