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

CVS2CL_AMFLAGS		=  $(CVS2CL_TAG) -U cvsusers.map --show-dead
CVS2CL_ALLFLAGS        :=  $(CVS2CL_AMFLAGS) $(CVS2CL_FLAGS)

confdir			=  $(sysconfdir)/vservers
confdefaultsdir		=  $(confdir)/.defaults
sbincfgdir		=  $(sbindir)
legacydir		=  $(pkglibdir)/legacy

include_HEADERS		=  $(lib_HDRS)
noinst_HEADERS		=  $(src_HDRS) $(lib_XHDRS) compat.h compat-c99.h

lib_LIBRARIES		=  $(lib_lib_LIBS)
lib_LTLIBRARIES		=  $(lib_lib_LTLIBS) $(src_lib_LTLIBS)
pkglib_LTLIBRARIES	=  $(src_pkglib_LTLIBS)

pkglib_SCRIPTS		=  $(scripts_pkglib_SCRPTS)
pkglib_DATA		=  $(scripts_pkglib_DTA)
pkglib_PROGRAMS		=  $(src_pkglib_PRGS)
legacy_SCRIPTS		=  $(scripts_legacy_SCRPTS)

sbincfg_DATA		=  $(scripts_sbincfg_DTA)

sbin_SCRIPTS		=  $(scripts_sbin_PRGS) $(src_sbin_SCRPTS)
sbin_PROGRAMS		=  $(src_sbin_PRGS)
noinst_PROGRAMS		=  $(test_PRGS)

initrd_SCRIPTS		=  $(sysv_SCRPTS)
sysconf_DATA		=  $(sysv_CFG)

DIETPROGS		=  $(src_DIETPROGS) $(tests_DIETPROGS) $(lib_DIETPROGS)

BUILT_SOURCES		=  linuxcaps.h pathconfig.h linuxvirtual.h

man_MANS		=  $(man_DATMAN)

EXTRA_PROGRAMS		=  $(src_EXTRA_PROGS)
EXTRA_DIST		=  $(sysv_XTRAS) $(sysv_CFG) \
                           $(doc_DAT) $(man_DATMAN) \
			   $(scripts_XTRAS) $(src_XTRAS) \
			   pathconfig.h.pathsubst \
			   util-vserver.spec \
			   THANKS

check_PROGRAMS		=  $(src_check_PRGS)
check_SCRIPTS		=  $(src_check_SCRPTS)
TESTS			=  $(src_TSTS)

TESTS_ENVIRONMENT	=  $(src_TESTS_ENV)

AM_CPPFLAGS		=  -I $(top_srcdir)/lib -D _GNU_SOURCE $(RELEASE_CPPFLAGS)

CLEANFILES		=  $(sysv_GENSCRPTS) $(scripts_CLEANFILES) $(BUILT_SOURCES) \
			   .*.pathsubst.stamp */.*.pathsubst.stamp \
			   linuxcaps.h pathconfig.h

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT	=  $(src_OPTIONS_EXEMPT) \
					   $(scripts_OPTIONS_EXEMPT) \
					   $(sysv_OPTIONS_EXEMPT)

if USE_DIETLIBC
DIET			=  diet
DIET_FLAGS		=
DIET_CC		       :=  $(DIET) $(DIET_FLAGS) $(CC)
DIET_LIBTOOL	       :=  $(LIBTOOL) --tag CC
VSERVER_LDFLAGS	       :=  -static
else
VSERVER_LDFLAGS	       :=
endif

linuxcaps.h:		${kernelincludedir}/linux/capability.h Makefile
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

pathsubst_RULES		=  s!@'PKGLIBDIR'@!$(pkglibdir)!g; \
			   s!@'SBINDIR'@!$(sbindir)!g; \
			   s!@'CONFDIR'@!$(confdir)!g; \
			   s!@'SYSCONFDIR'@!$(sysconfdir)!g; \
			   s!@'PKGSTATEDIR'@!$(pkgstatedir)!g; \
			   s!@'PKGSTATEREVDIR'@!$(pkgstaterevdir)!g; \
			   s!@'VSERVERDIR'@!$(vserverdir)!g; \
			   s!@'VSERVERPKGDIR'@!$(vserverpkgdir)!g; \
			   s!@'VSERVERPKGDIR'@!$(vserverpkgdir)!g; \
			   s!@'INITRDDIR'@!$(initrddir)!g; \
			   s!@'LIBDIR'@!$(libdir)!g; \
			   s!@'INCLUDEDIR'@!$(includedir)!g; \
			   s!@'PACKAGE_BUGREPORT'@!$(PACKAGE_BUGREPORT)!g; \
			   s!@'PACKAGE_NAME'@!$(PACKAGE_NAME)!g; \
			   s!@'PACKAGE_STRING'@!$(PACKAGE_STRING)!g; \
			   s!@'PACKAGE_TARNAME'@!$(PACKAGE_TARNAME)!g; \
			   s!@'PACKAGE_VERSION'@!$(PACKAGE_VERSION)!g; \
			   s!@'AWK'@!$(AWK)!g; \
			   s!@'GREP'@!$(GREP)!g; \
			   s!@'IP'@!$(IP)!g; \
			   s!@'MOUNT'@!$(MOUNT)!g; \
			   s!@'UMOUNT'@!$(UMOUNT)!g; \
			   s!@'NICE'@!$(NICE)!g; \
			   s!@'VCONFIG'@!$(VCONFIG)!g; \
			   s!@'WC'@!$(WC)!g

pathconfig.h:		.pathconfig.h.pathsubst.stamp
.%.pathsubst.stamp:	%.pathsubst Makefile
			@mkdir -p $$(dirname '$@')
			@echo "sed -e '...' $*.pathsubst >$*"
			@sed -e '$(pathsubst_RULES)' '$<' >'$@.tmp'
			@-chmod --reference='$<' '$@.tmp'
			@if cmp -s '$@.tmp' '$*'; then \
				echo "... no changes, reusing old version"; \
				rm -f '$@.tmp'; \
			else \
				rm -f '$*'; mv '$@.tmp' '$*'; \
			fi
			@chmod a-w '$*'
			@touch '$@'

pkgstatedir		=  $(localstatedir)/run/vservers
pkgstaterevdir		=  $(pkgstatedir).rev
vserverpkgdir		=  $(vserverdir)/.pkg

install-data-hook:
			$(mkinstalldirs) -m 755 $(DESTDIR)$(pkgstatedir) $(DESTDIR)$(pkgstaterevdir)
			$(mkinstalldirs) -m 755 $(DESTDIR)$(confdefaultsdir)/{apps/pkgmgmt,files}
			f=$(DESTDIR)$(confdefaultsdir)/vdirbase; test -e "$$f" || ln -s '$(vserverdir)'     "$$f"
			f=$(DESTDIR)$(confdefaultsdir)/run.rev;  test -e "$$f" || ln -s '$(pkgstaterevdir)' "$$f"

install-vserverdir:
			$(mkinstalldirs) $(DESTDIR)$(vserverdir) $(DESTDIR)$(vserverpkgdir)

if USE_DIETLIBC
$(addsuffix $(EXEEXT), $(DIETPROGS)) \
$(addsuffix $(OBJEXT), $(DIETPROGS)) \
		:		LIBTOOL=$(DIET_LIBTOOL)

$(addsuffix $(EXEEXT), $(DIETPROGS)) \
$(addsuffix $(OBJEXT), $(DIETPROGS)) \
		:		CC=$(DIET_CC)
endif

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
include $(top_srcdir)/m4/pkgconfig.am
