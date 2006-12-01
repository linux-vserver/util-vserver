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

AC_DEFUN([ENSC_CHECK_EXT2FS_HEADER],
[
    AC_LANG_PUSH(C++)
    AC_CACHE_CHECK([for ext2fs-headers], [ensc_cv_test_ext2fs_header],[
	AC_COMPILE_IFELSE(AC_LANG_SOURCE([#include <ext2fs/ext2_fs.h>
					  int main() { return 0; }]),
			  [ensc_cv_test_ext2fs_header=e2fsprogs],[
	AC_COMPILE_IFELSE(AC_LANG_SOURCE([#include <linux/ext2_fs.h>
					  int main() { return 0; }]),
			  [ensc_cv_test_ext2fs_header=kernel],[
	ensc_cv_test_ext2fs_header=none])])])

    case x"$ensc_cv_test_ext2fs_header" in
	(xe2fsprogs)
		AC_CHECK_HEADER([ext2fs/ext2_fs.h],
			[AC_DEFINE(ENSC_HAVE_EXT2FS_EXT2_FS_H, 1, [define when <ext2fs/ext2_fs.h> is usable])],
			[AC_MSG_FAILURE([unexpected error while checkin for <ext2fs/ext2_fs.h>])])
		;;
	(xkernel)
		AC_CHECK_HEADER([linux/ext2_fs.h],
			[AC_DEFINE(ENSC_HAVE_LINUX_EXT2_FS_H, 1, [define when <linux/ext2_fs.h> is usable])],
			[AC_MSG_FAILURE([unexpected error while checkin for <linux/ext2_fs.h>])])
		;;
	(*)
		AC_MSG_FAILURE([
ext2fs headers were not found, or they are not usable. This can have
the following reasons:

* you have neither the e2fsprogs nor the kernel headers installed

* kernel headers are broken (e.g. these of linux 2.6 are known to be)
  and you do not have e2fsprogs headers installed; please try to install
  - e2fsprogs-devel (for Red Hat), or
  - lib*ext2fs2-devel (for Mandriva), or
  - e2fslibs-dev (for Debian)
  in this case.

* kernel headers are broken and your e2fsprogs headers are too old;
  until version 1.27 (inclusive), they are using reserved C++ keywords

* kernel headers are broken and your e2fsprogs headers are too new;
  recent (January 2004) BK snapshots of e2fsprogs are unusable for
  C++, for details and a solution see
  https://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=112448


In the latter two cases you have the following options:
* fix the headers manually, or
* install a stable version of e2fsprogs (e.g. 1.34), or
* use good kernel headers (from linux 2.4.x)
])
		;;
    esac
    AC_LANG_POP
])
