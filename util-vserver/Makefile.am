## $Id$

# Copyright (C) 2003,2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

include_HEADERS =	$(lib_HDRS)
noinst_HEADERS =	$(src_HDRS) $(lib_XHDRS) \
			$(lib_internal_libinternal_a_XHDRS) \
			$(ENSC_VECTOR_HDRS) $(ENSC_VECTOR_XHDRS) \
			$(ENSC_WRAPPERS_HDRS) \
			compat.h compat-c99.h

noinst_LIBRARIES	=  $(src_lib_LIBS) $(ENSC_VECTOR_LIBS) $(lib_internal_LIBS)
lib_LIBRARIES		=  $(lib_lib_LIBS)
lib_LTLIBRARIES		=  $(lib_lib_LTLIBS) $(src_lib_LTLIBS)
pkglib_LTLIBRARIES	=  $(src_pkglib_LTLIBS)

pkglib_SCRIPTS		=  $(scripts_pkglib_SCRPTS)
pkglib_DATA		=  $(scripts_pkglib_DTA) \
			   FEATURES.txt
pkglib_PROGRAMS =	$(src_pkglib_PRGS)
legacy_SCRIPTS =	$(scripts_legacy_SCRPTS) $(src_legacy_SCRPTS)
legacy_PROGRAMS =	$(src_legacy_PRGS)

sbincfg_DATA		=  $(scripts_sbincfg_DTA)

sbin_SCRIPTS		=  $(scripts_sbin_PRGS) $(src_sbin_SCRPTS)
sbin_PROGRAMS		=  $(src_sbin_PRGS)
noinst_PROGRAMS		=  $(test_PRGS)
noinst_DATA		=  $(contrib_DTA) $(doc_DAT)

initrd_SCRIPTS =
sysconf_DATA =

DIETPROGS =		$(src_DIETPROGS) $(tests_DIETPROGS) $(lib_DIETPROGS) \
			$(lib_internal_DIETPROGS)
			$(ENSC_VECTOR_LIBS)

BUILT_SOURCES =		linuxcaps.h pathconfig.h linuxvirtual.h

man_MANS =		$(man_DATMAN)

EXTRA_PROGRAMS =	$(src_EXTRA_PROGS)
EXTRA_DIST =            $(doc_DAT) $(man_DATMAN) \
			$(scripts_XTRAS) $(src_XTRAS) \
			$(contrib_XTRAS) \
			pathconfig.h.pathsubst \
			util-vserver.spec \
			THANKS

check_PROGRAMS =	$(src_check_PRGS) $(lib_testsuite_PRGS) \
			$(ENSC_VECTOR_TESTSUITE_PRGS)
check_SCRIPTS =		$(src_check_SCRPTS)
TESTS =			$(src_TSTS) $(lib_testsuite_TSTS) \
			$(ENSC_VECTOR_TESTSUITE_TSTS)

TESTS_ENVIRONMENT	=  $(src_TESTS_ENV)

AM_CPPFLAGS		=  -I $(top_srcdir)/lib -I $(top_srcdir)/ensc_wrappers -D _GNU_SOURCE $(RELEASE_CPPFLAGS)

CLEANFILES =		$(scripts_CLEANFILES) $(BUILT_SOURCES) \
			$(contrib_CLEANFILES) \
			.*.pathsubst.stamp */.*.pathsubst.stamp */*/.*.pathsubst.stamp \
			linuxcaps.h pathconfig.h

DISTCLEANFILES =	FEATURES.txt


AM_INSTALLCHECK_STD_OPTIONS_EXEMPT	=  $(src_OPTIONS_EXEMPT) \
					   $(scripts_OPTIONS_EXEMPT)

LIBTOOL_CC =		@LIBTOOL@ --tag CC
LIBTOOL_CXX =		@LIBTOOL@ --tag CXX

if USE_DIETLIBC
DIET			=  diet
DIET_FLAGS		=
DIET_CC		       :=  $(DIET) $(DIET_FLAGS) $(CC)
DIET_LIBTOOL	       :=  $(LIBTOOL_CC)
VSERVER_LDFLAGS	       :=  -static
else
VSERVER_LDFLAGS	       :=
endif

if USE_DIETLIBC_COMPAT
DIET_COMPAT_LDADDS =	-lcompat
else
DIET_COMPAT_LDADDS =
endif

VSERVER_LDADDS =	lib/libvserver.la  $(DIET_COMPAT_LDADDS)


linuxcaps.h:		${kernelincludedir}/linux/capability.h Makefile
			@rm -f $@
			echo '#include <stdint.h>' >$@.tmp
			$(SED) -e 's!^#include .*!!g;s!\<__u32\>!uint32_t!g;s!\<__user\>!!g;' $< >>$@.tmp
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
			   s!@'LEGACYDIR'@!$(legacydir)!g; \
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
			   s!@'MANDIR'@!$(mandir)!g; \
			   s!@'INCLUDEDIR'@!$(includedir)!g; \
			   s!@'PACKAGE'@!$(PACKAGE)!g; \
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
			   s!@'PS'@!$(PS)!g; \
			   s!@'WC'@!$(WC)!g; \
			   s!@'WGET'@!$(WGET)!g; \
			   s!@'ENSC_HAVE_C99_COMPILER_TRUE'@!\@ENSC_HAVE_C99_COMPILER_TRUE@ !g; \
			   s!@'ENSC_HAVE_C99_COMPILER_FALSE'@!\@ENSC_HAVE_C99_COMPILER_FALSE@ !g; \
			   s!@'ENSC_HAVE_CXX_COMPILER_TRUE'@!\@ENSC_HAVE_CXX_COMPILER_TRUE@ !g; \
			   s!@'ENSC_HAVE_CXX_COMPILER_FALSE'@!\@ENSC_HAVE_CXX_COMPILER_FALSE@ !g;

pathconfig.h:		.pathconfig.h.pathsubst.stamp
.%.pathsubst.stamp:	%.pathsubst Makefile
			@mkdir -p $$(dirname '$@')
			@echo "$(SED) -e '...' $*.pathsubst >$*"
			@$(SED) -e '$(pathsubst_RULES)' '$<' >'$@.tmp'
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

fix_SCRPTS		=  $(addprefix $(pkglibdir)/, $(notdir $(pkglib_SCRIPTS))) \
			   $(addprefix $(legacydir)/, $(notdir $(legacy_SCRIPTS))) \
			   $(addprefix $(sbindir)/,   $(notdir $(sbin_SCRIPTS))) \
			   $(addprefix $(initrddir)/, $(notdir $(initrd_SCRIPTS)))

install-data-hook:	install-fix-script-paths install-create-dirs

install-create-dirs:
			$(mkinstalldirs) -m 755 $(DESTDIR)$(pkgstatedir) $(DESTDIR)$(pkgstaterevdir)
			$(mkinstalldirs) -m 755 $(DESTDIR)$(confdefaultsdir)/{apps/pkgmgmt,files}
			f=$(DESTDIR)$(confdefaultsdir)/vdirbase; test -e "$$f" || ln -sf '$(vserverdir)'     "$$f"
			f=$(DESTDIR)$(confdefaultsdir)/run.rev;  test -e "$$f" || ln -sf '$(pkgstaterevdir)' "$$f"

install-fix-script-paths:
			test "/usr/lib/util-vserver" = "$(pkglibdir)" || \
			for i in $(fix_SCRPTS); do \
				f="$(DESTDIR)$$i"; \
				$(SED) -e 's!/usr/lib/util-vserver!$(pkglibdir)!g' "$$f" >"$$f.tmp"; \
				$(CMP) -s "$$f.tmp" "$$f" || cat "$$f.tmp" >"$$f"; \
				rm -f "$$f.tmp"; \
			done

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
include $(top_srcdir)/contrib/Makefile-files
include $(top_srcdir)/ensc_vector/Makefile-files
include $(top_srcdir)/ensc_wrappers/Makefile-files
include $(top_srcdir)/lib_internal/Makefile-files

include $(top_srcdir)/m4/gpgsig.am
include $(top_srcdir)/m4/validate.am
include $(top_srcdir)/m4/changelog.am
include $(top_srcdir)/m4/pkgconfig.am
