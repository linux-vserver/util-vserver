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

AC_DEFUN([_ENSC_DIETLIBC_C99],
[
	AH_TEMPLATE([ENSC_DIETLIBC_C99], [Define to 1 if dietlibc supports C99])

	AC_CACHE_CHECK([whether dietlibc supports C99], [ensc_cv_c_dietlibc_c99],
	[
		_ensc_dietlibc_c99_old_CFLAGS=$CFLAGS
		_ensc_dietlibc_c99_old_CC=$CC

		CFLAGS="-std=c99"
		CC="${DIET:-diet} $CC"

		AC_LANG_PUSH(C)
		AC_COMPILE_IFELSE([/* */],[
			AC_COMPILE_IFELSE([
				#include <stdint.h>
				#include <sys/cdefs.h>
				#if defined(inline)
				#  error 'inline' badly defined
				#endif
				volatile uint64_t	a;
			],
			[ensc_cv_c_dietlibc_c99=yes],
			[ensc_cv_c_dietlibc_c99=no])],
			[ensc_cv_c_dietlibc_c99='skipped (compiler does not support C99)'])
		AC_LANG_POP

		CC=$_ensc_dietlibc_c99_old_CC
		CFLAGS=$_ensc_dietlibc_c99_old_CFLAGS
	])

	if test x"$ensc_cv_c_dietlibc_c99" = xyes; then
		AC_DEFINE(ENSC_DIETLIBC_C99,1)
	fi
])

dnl Usage: ENSC_ENABLE_DIETLIBC(<conditional>)
dnl        <conditional> ... automake-conditional which will be set when
dnl                          dietlibc shall be enabled

AC_DEFUN([ENSC_ENABLE_DIETLIBC],
[
	AC_MSG_CHECKING([whether to enable dietlibc])
	AC_ARG_ENABLE([dietlibc],
		      [AC_HELP_STRING([--disable-dietlibc],
				      [do not use dietlibc (default: use dietlibc)])],
		      [case "$enableval" in
			  yes)	use_dietlibc=forced;;
			  no)	use_dietlibc=forced_no;;
			  *)	AC_MSG_ERROR(['$enableval' is not a valid value for --enable-dietlibc]);;
		       esac],
		      [: ${DIET:=diet}
		       which "$DIET" >/dev/null 2>/dev/null && use_dietlibc=detected || use_dietlibc=detected_no])

	ensc_have_dietlibc=no
	case x"$use_dietlibc" in
	    xdetected)
		AM_CONDITIONAL($1, true)
		AC_MSG_RESULT([yes (autodetected)])
		ensc_have_dietlibc=yes
		;;
	    xforced)
		AM_CONDITIONAL($1, true)
		AC_MSG_RESULT([yes (forced)])
		ensc_have_dietlibc=yes
		;;
	    xdetected_no)
		AM_CONDITIONAL($1, false)
		AC_MSG_RESULT([no (detected)])
		;;
	    xforced_no)
		AM_CONDITIONAL($1, false)
		AC_MSG_RESULT([no (forced)])
		;;
	    *)
		AC_MSG_ERROR([internal error, use_dietlibc was "$use_dietlibc"])
		;;
	esac

	if test x"$ensc_have_dietlibc" != xno; then
		_ENSC_DIETLIBC_C99
	fi
])
