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

dnl Usage: ENSC_SYSCALLNR(<syscall>,<default>)

AC_DEFUN([ENSC_SYSCALLNR],
[
	AC_REQUIRE([AC_PROG_CPP])
	AC_REQUIRE([AC_PROG_EGREP])
	AC_REQUIRE([ENSC_KERNEL_HEADERS])

	AC_CACHE_CHECK([for number of syscall '$1'], [ensc_cv_value_syscall_$1],
	[
		AC_LANG_PUSH(C)
		AC_LANG_CONFTEST([
#include <asm/unistd.h>
#ifdef __NR_$1
ensc_syscall_tmp_nr=__NR_$1;
ensc_syscall_tmp_src=ENSC_MARK
#endif
])
		ensc_syscall_tmp_nr=
		ensc_syscall_tmp_src=
		test "$ensc_syscall_tmp_nr" || \
			eval $($CPP $CPPFLAGS -D ENSC_MARK='glibc'                       conftest.c | $EGREP '^ensc_syscall_tmp_(nr=[[1-9]][[0-9]]*;|src=.*)$')
		test "$ensc_syscall_tmp_nr" || \
			eval $($CPP $CPPFLAGS -D ENSC_MARK='kernel' -I $kernelincludedir conftest.c | $EGREP '^ensc_syscall_tmp_(nr=[[1-9]][[0-9]]*;|src=.*)$')
		test "$ensc_syscall_tmp_nr" || {
			ensc_syscall_tmp_nr=$2
			ensc_syscall_tmp_src=default
		}

		if test x"$ensc_syscall_tmp_nr" = x; then
			AC_MSG_ERROR(
[Can not determine value of __NR_$1; please verify your glibc/kernelheaders, and/or set CPPFLAGS='-D=__NR_$1=<value>' environment when calling configure.])
		fi
		AC_LANG_POP

		ensc_cv_value_syscall_$1="$ensc_syscall_tmp_nr/$ensc_syscall_tmp_src"
	])

	ensc_syscall_tmp_nr=${ensc_cv_value_syscall_$1%/*}
	ensc_syscall_tmp_src=${ensc_cv_value_syscall_$1#*/}

	if test x"$ensc_syscall_tmp_src" != x'glibc'; then
		AC_DEFINE_UNQUOTED(ENSC_SYSCALL__NR_$1, $ensc_syscall_tmp_nr, [The number of the $1 syscall])
	fi
])
