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
		ensc_dietlibc_need_compat_old_CC=$CC
		CC="${DIET:-diet} $CC"
		AC_LINK_IFELSE([
			AC_LANG_PROGRAM([
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>

#define __NR_foo0		42
#define __NR_foo1		42
#define __NR_foo2		42
#define __NR_foo3		42
#define __NR_foo4		42
#define __NR_foo5		42
inline static _syscall0(int, foo0)
inline static _syscall1(int, foo1, int, a)
inline static _syscall2(int, foo2, int, a, int, b)
inline static _syscall3(int, foo3, int, a, int, b, int, c)
inline static _syscall4(int, foo4, int, a, int, b, int, c, int, d)
inline static _syscall5(int, foo5, int, a, int, b, int, c, int, d, int, e)
],
			[foo0(); foo1(0); foo2(0,0); foo3(0,0,0); foo4(0,0,0,0); foo5(0,0,0,0,0);])],
			[ensc_cv_c_dietlibc_compat=no],
			[ensc_cv_c_dietlibc_compat=yes])
		CC=$ensc_dietlibc_need_compat_old_CC
		AC_LANG_POP
	])

	if test x"$1" != x; then
		AM_CONDITIONAL($1, test x"$ensc_cv_c_dietlibc_compat" = xyes)
	fi
])
