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

dnl Usage: ENSC_SET_SEARCHPATH(<PATH>)
AC_DEFUN([ENSC_SET_SEARCHPATH],
[
	ensc_searchpath="$1"
])

dnl Usage: ENSC_PATHPROG_INIT
AC_DEFUN([ENSC_PATHPROG_INIT],
[
	ENSC_PATHPROG_SED=
	AC_SUBST([ENSC_PATHPROG_SED])
])


dnl Usage: ENSC_PATHPROG(<VAR>, <PROG>[, <DFLT>, <DESCR>, <DEREF>])
AC_DEFUN([ENSC_PATHPROG],
[
	AC_REQUIRE([ENSC_SET_SEARCHPATH])
	AC_REQUIRE([ENSC_PATHPROG_INIT])

	if test -z "$3"; then
		rq=true
	else
		rq=false
	fi

	if $rq; then
		ensc_dflt=
	else
		ensc_dflt="$3"
	fi

	AC_PATH_PROGS($1, [$2], [$ensc_dflt], [$ensc_searchpath])

	if test -z "${$1}" && $rq; then
		if test -z "$4"; then
			AC_MSG_ERROR([Can not find the '$2' tool within '${ensc_searchpath:-$PATH}'.])
		else
			AC_MSG_ERROR([
Can not find the '$2' tool within '${ensc_searchpath:-$PATH}'.
$4])
		fi
	fi

	if test "x$5" = x; then
		if test -h "${$1}"; then
			$1=`readlink -f "${$1}"`
		fi
	fi

	test "${$1}" && ENSC_PATHPROG_SED="${ENSC_PATHPROG_SED}s!@'$1'@!${$1}!g;"

	test "${$1}"])



dnl Usage: ENSC_PATHPROG_STANDARD_TOOLS)
AC_DEFUN([ENSC_PATHPROG_STANDARD_TOOLS],
[
	ENSC_PATHPROG(AWK,       awk)
	ENSC_PATHPROG(CAT,       cat)
	ENSC_PATHPROG(CHOWN,     chown)
	ENSC_PATHPROG(CMP,       cmp)
	ENSC_PATHPROG(CP,        cp)
	ENSC_PATHPROG(DIRNAME,   dirname)
	ENSC_PATHPROG(EGREP,     egrep, [], [], no-deref)
	ENSC_PATHPROG(ENV,       env)
	ENSC_PATHPROG(GREP,      grep)
	ENSC_PATHPROG(LN,        ln)
	ENSC_PATHPROG(MKDIR,     mkdir)
	ENSC_PATHPROG(MKFIFO,    mkfifo)
	ENSC_PATHPROG(MKTEMP,    mktemp)
	ENSC_PATHPROG(MOUNT,     mount)
	ENSC_PATHPROG(MV,        mv)
	ENSC_PATHPROG(NICE,      nice)
	ENSC_PATHPROG(PS,        ps)
	ENSC_PATHPROG(RM,        rm)
	ENSC_PATHPROG(RMDIR,     rmdir)
	ENSC_PATHPROG(SED,       sed)
	ENSC_PATHPROG(SH,        sh)
	ENSC_PATHPROG(TAC,       tac)
	ENSC_PATHPROG(TAR,       tar)
	ENSC_PATHPROG(TOUCH,     touch)
	ENSC_PATHPROG(TTY,       tty)
	ENSC_PATHPROG(UMOUNT,    umount)
	ENSC_PATHPROG(WC,        wc)
])
	
