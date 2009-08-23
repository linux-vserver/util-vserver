dnl $Id$

dnl Copyright (C) 2002, 2003, 2009
dnl     Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; version 2 and/or 3 of the License.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

dnl Usage: ENSC_CHANGELOG()

AC_DEFUN([ENSC_CHANGELOG],
[
	AC_CHECK_PROGS(SVN2CL, [svn2cl])
	AM_CONDITIONAL(HAVE_SVN2CL,  [test x"$SVN2CL" != x])
])
