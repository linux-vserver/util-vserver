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

CVS2CL_AMFLAGS          =  $(CVS2CL_TAG) -U cvsusers.map --show-dead
CVS2CL_ALLFLAGS        :=  $(CVS2CL_AMFLAGS) $(CVS2CL_FLAGS)

sysvdir			=  $(sysconfdir)/init.d
vservercfgdir		=  $(sysconfdir)/vservers

include_HEADERS		=  $(lib_HDRS)
noinst_HEADERS		=  $(src_HDRS) $(lib_XHDRS) compat.h

lib_LIBRARIES		=  $(lib_lib_LIBS)

pkglib_SCRIPTS		=  $(distrib_SCRPTS) $(scripts_SCRPTS) $(src_pkglib_SCRPTS)
pkglib_DATA		=  $(distrib_DAT) $(scriptscfg_DATA)
pkglib_PROGRAMS		=  $(src_pkglib_PRGS)

sbin_SCRIPTS		=  $(scripts_PRGS) $(linuxconf_SCRPTS)
sbin_PROGRAMS		=  $(src_sbin_PRGS)
noinst_PROGRAMS		=  $(test_PRGS)

sysv_SCRIPTS		=  $(sysv_SCRPTS)
sysconf_DATA		=  $(sysv_CFG)
vservercfg_DATA		=  $(linuxconf_cfg_DTA)

BUILT_SOURCES		=  linuxcaps.h linuxvirtual.h

man_MANS		=  $(man_DATMAN) $(linuxconf_DATMAN)

m4_FILES		=  m4/ensc_cflags.m4 m4/ensc_kerneldir.m4 \
			   m4/ensc_changelog.m4 m4/ensc_uv_vrootdir.m4

EXTRA_DIST		=  $(distrib_SCRPTS) $(distrib_DAT) \
			   $(sysv_XTRAS) $(sysv_CFG) \
                           $(doc_DAT) $(man_DATMAN) $(linuxconf_XTRAS) \
			   $(scripts_SCRPTS) $(scripts_XTRAS) \
			   $(m4_FILES) \
			   util-vserver.spec THANKS

AM_CPPFLAGS		=  -I $(top_srcdir)/lib -D VERSION=\"$(VERSION)\" -D PKGLIBDIR=\"$(pkglibdir)\" -D _GNU_SOURCE
AM_CFLAGS		=  -Wall -pedantic

CLEANFILES		=  $(sysv_GENSCRPTS) $(scripts_GENSCRPTS) \
			   $(linuxconf_GENFILES) \
			   $(BUILT_SOURCES)


linuxcaps.h:	${kernelincludedir}/linux/capability.h Makefile
		@rm -f $@
		echo '#include <stdint.h>' >$@.tmp
		sed -e 's!^#include .*!!g;s!\<__u32\>!uint32_t!g;s!\<__user\>!!g;' $< >>$@.tmp
		@-chmod --reference $< $@.tmp &>/dev/null
		mv -f $@.tmp $@
		@chmod a-w $@

linuxvirtual.h:
if ENSC_ENABLE_INTERNAL_HEADERS
		ln -sf '$(top_srcdir)/lib/virtual.h' '$@'
else
		ln -sf '$(kernelincludedir)/linux/virtual.h' '$@' && test -e '$@' || \
		ln -sf '$(top_srcdir)/lib/virtual.h' '$@'
endif

include $(top_srcdir)/lib/Makefile-files
include $(top_srcdir)/src/Makefile-files
include $(top_srcdir)/distrib/Makefile-files
include $(top_srcdir)/doc/Makefile-files
include $(top_srcdir)/man/Makefile-files
include $(top_srcdir)/scripts/Makefile-files
include $(top_srcdir)/tests/Makefile-files
include $(top_srcdir)/sysv/Makefile-files
include $(top_srcdir)/linuxconf/Makefile-files

include $(top_srcdir)/m4/gpgsig.am
include $(top_srcdir)/m4/validate.am
include $(top_srcdir)/m4/changelog.am
