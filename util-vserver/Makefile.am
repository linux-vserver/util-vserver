## $Id$

# Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#  

ACLOCAL_AMFLAGS		=  -I m4
SUBDIRS			=  . distrib

sysvdir			=  $(sysconfdir)/init.d
confdir			=  $(sysconfdir)/vservers

include_HEADERS		=  $(lib_HDRS)
noinst_HEADERS		=  $(src_HDRS)

lib_LIBRARIES		=  $(lib_lib_LIBS)

pkglib_SCRIPTS		=  $(scripts_SCRPTS)
pkglib_DATA		=  $(scriptscfg_DATA)
pkglib_PROGRAMS		=  $(src_pkglib_PRGS)

sbin_SCRIPTS		=  $(scripts_PRGS)
sbin_PROGRAMS		=  $(src_sbin_PRGS)
noinst_PROGRAMS		=  $(test_PRGS)

sysv_SCRIPTS		=  $(sysv_SCRPTS)
sysconf_DATA		=  $(sysv_CFG)

BUILT_SOURCES		=  linuxcaps.h pathconfig.h

man_MANS		=  $(man_DATMAN)

EXTRA_DIST		=  $(sysv_XTRAS) $(sysv_CFG) \
                           $(doc_DAT) $(man_DATMAN) \
			   $(scripts_SCRPTS) $(scripts_PRGS) $(scripts_XTRAS) \
			   pathconfig.h.pathsubst \
			   vserver.spec \
                           m4/ensc_cflags.m4 \
			   THANKS

AM_CPPFLAGS		=  -I $(top_srcdir)/lib -D _GNU_SOURCE
AM_CFLAGS		=  -Wall -pedantic

CLEANFILES		=  $(sysv_GENSCRPTS) $(scripts_GENSCRPTS) \
			   linuxcaps.h pathconfig.h

if USE_DIETLIBC
DIET			=  diet
DIET_FLAGS		=
DIET_CC		       :=  $(DIET) $(DIET_FLAGS) $(CC)
endif

linuxcaps.h:
			ln -sf ${kernelincludedir}/linux/capability.h linuxcaps.h

%:			%.pathsubst Makefile
			@mkdir -p $$(dirname '$@')
			sed -e 's!@'PKGLIBDIR'@!$(pkglibdir)!g; \
				s!@'SBINDIR'@!$(sbindir)!g; \
				s!@'CONFDIR'@!$(confdir)!g; \
				s!@'SYSCONFDIR'@!$(sysconfdir)!g' '$<' >'$@.tmp'
			if cmp -s '$@.tmp' '$@'; then rm -f '$@.tmp'; else rm -f '$@'; mv '$@.tmp' '$@'; fi
			@chmod a-w '$@'


include $(top_srcdir)/lib/Makefile-files
include $(top_srcdir)/src/Makefile-files
include $(top_srcdir)/doc/Makefile-files
include $(top_srcdir)/man/Makefile-files
include $(top_srcdir)/scripts/Makefile-files
include $(top_srcdir)/tests/Makefile-files
include $(top_srcdir)/sysv/Makefile-files

include $(top_srcdir)/m4/gpgsig.am
include $(top_srcdir)/m4/validate.am
include $(top_srcdir)/m4/changelog.am
