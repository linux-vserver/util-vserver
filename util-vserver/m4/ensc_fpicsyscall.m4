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

AC_DEFUN([ENSC_FPIC_SYSCALL],
[
    AC_CACHE_CHECK([whether syscall() allows -fpic], [ensc_cv_c_fpic_syscall],
    [
        old_CFLAGS=$CFLAGS
        CFLAGS="-fPIC -DPIC"

	AC_LANG_PUSH(C)
        AC_COMPILE_IFELSE([
            #include <sys/syscall.h>
            #include <unistd.h>
            #include <asm/unistd.h>
            #include <errno.h>
            
            #define __NR_dummy	42
            _syscall3(int, dummy, int, a, int, b, int, c)],
        [ensc_cv_c_fpic_syscall=yes], [ensc_cv_c_fpic_syscall=no])
	AC_LANG_POP

        CFLAGS=$old_CFLAGS
    ])
    
    AM_CONDITIONAL(ENSC_ALLOW_FPIC_WITH_SYSCALL, [test x"$ensc_cv_c_fpic_syscall" = xyes])
])
