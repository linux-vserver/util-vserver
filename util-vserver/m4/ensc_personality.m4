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

dnl Usage: ENSC_CHECK_PERSONALITY_FLAGS
AC_DEFUN([ENSC_CHECK_PERSONALITY_FLAGS],
[
	AC_MSG_CHECKING([for declarations in <linux/personality.h>])
	AC_MSG_RESULT([])

	AC_LANG_PUSH(C)
	AC_CHECK_DECLS([MMAP_PAGE_ZERO, ADDR_LIMIT_32BIT, SHORT_INODE,
		        WHOLE_SECONDS, STICKY_TIMEOUTS, ADDR_LIMIT_3GB],
		       [],[],
		       [
#include <linux/personality.h>
			])

	AC_CHECK_DECLS([PER_LINUX, PER_LINUX_32BIT, PER_SVR4, PER_SVR3,
                        PER_SCOSVR3, PER_OSR5, PER_WYSEV386, PER_ISCR4,
			PER_BSD, PER_SUNOS, PER_XENIX, PER_LINUX32,
			PER_LINUX32_3GB, PER_IRIX32, PER_IRIXN32,
			PER_IRIX64, PER_RISCOS, PER_SOLARIS, PER_UW7,
			PER_HPUX, PER_OSF4],
		       [], [],
		       [
#include <linux/personality.h>
			])
	AC_LANG_POP
])

