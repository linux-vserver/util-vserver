## $Id$

# Copyright (C) 2003,2004,2005,2006 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

confdir			=  $(sysconfdir)/vservers
confdefaultsdir		=  $(confdir)/.defaults
sbincfgdir		=  $(sbindir)
legacydir		=  $(pkglibdir)/legacy

include_HEADERS =
noinst_HEADERS =	$(ENSC_VECTOR_HDRS) \
			compat.h compat-c99.h

noinst_LIBRARIES =	$(ENSC_VECTOR_LIBS)
lib_LIBRARIES =
lib_LTLIBRARIES =
pkglib_LTLIBRARIES =

pkglib_SCRIPTS =
pkglib_DATA =		FEATURES.txt
pkglib_PROGRAMS =
legacy_SCRIPTS =
legacy_PROGRAMS =

sbin_SCRIPTS =
sbin_PROGRAMS =
noinst_PROGRAMS =
noinst_DATA =

initrd_SCRIPTS =
sysconf_DATA =

pyexec_LTLIBRARIES = 
pyexec_SCRIPTS =

DIETPROGS =		$(LIBENSCVECTOR_DIET)

BUILT_SOURCES =		pathconfig.h \
			.fixups

man_MANS =

kernel_HDRS = \
	$(patsubst ${srcdir}/%,%,$(wildcard ${srcdir}/kernel/*.h))


EXTRA_PROGRAMS =
EXTRA_DIST =		pathconfig.h.pathsubst \
			util-vserver.spec \
			THANKS REQUIREMENTS \
			$(kernel_HDRS)

check_PROGRAMS =
check_SCRIPTS =

TESTS_DEBUG =		no
TESTS =
TESTS_ENVIRONMENT =	ensc_use_expensive_tests="$(ENSC_USE_EXPENSIVE_TESTS)" \
			ensc_test_debug="$(TESTS_DEBUG)"

AM_CPPFLAGS =		-I $(top_srcdir)/lib -I $(top_srcdir)/ensc_wrappers \
			-D_GNU_SOURCE -D_REENTRANT $(RELEASE_CPPFLAGS)

CLEANFILES =		$(BUILT_SOURCES) \
			.*.pathsubst.stamp */.*.pathsubst.stamp */*/.*.pathsubst.stamp \
			pathconfig.h .fixups

DISTCLEANFILES =	FEATURES.txt


AM_INSTALLCHECK_STD_OPTIONS_EXEMPT =

if USE_DIETLIBC
DIET_CC		       :=  $(DIET) $(DIETFLAGS) $(CC)
VSERVER_LDFLGS	       :=  -static
else
VSERVER_LDFLGS	       :=
endif

if USE_DIETLIBC_COMPAT
DIET_COMPAT_LDADDS =	-lcompat
else
DIET_COMPAT_LDADDS =
endif

VSERVER_LDADDS =	$(LIBVSERVER) $(DIET_COMPAT_LDADDS)

silent.%:		FORCE
			@t=$$($(MKTEMP) -d /tmp/build.XXXXXX) && \
			trap "rm -rf $$t" EXIT && \
			mkfifo $$t/fifo && \
			( grep --line-buffered -v \
				     -e ': In function' \
				     -e 'your code still has assertions enabled' \
				     -e ' is obsoleted' \
				     -e 'warning: #warning' <$$t/fifo & ) && \
			exec 2>$$t/fifo && \
			$(MAKE) '$*' -s >/dev/null

pathsubst_RULES =	s!@'PKGLIBDIR'@!$(pkglibdir)!g; \
			s!@'LEGACYDIR'@!$(legacydir)!g; \
			s!@'SBINDIR'@!$(sbindir)!g; \
			s!@'CONFDIR'@!$(confdir)!g; \
			s!@'SYSCONFDIR'@!$(sysconfdir)!g; \
			s!@'PKGSTATEDIR'@!$(pkgstatedir)!g; \
			s!@'PKGCACHEDIR'@!$(pkgcachedir)!g; \
			s!@'PKGSTATEREVDIR'@!$(pkgstaterevdir)!g; \
			s!@'VSERVERDIR'@!$(vserverdir)!g; \
			s!@'VSERVERPKGDIR'@!$(vserverpkgdir)!g; \
			s!@'VSERVERPKGDIR'@!$(vserverpkgdir)!g; \
			s!@'VSHELPERSTATEDIR'@!$(vshelperstatedir)!g; \
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
			s!@'ENSC_HAVE_C99_COMPILER_TRUE'@!\@ENSC_HAVE_C99_COMPILER_TRUE@ !g; \
			s!@'ENSC_HAVE_C99_COMPILER_FALSE'@!\@ENSC_HAVE_C99_COMPILER_FALSE@ !g; \
			s!@'ENSC_HAVE_CXX_COMPILER_TRUE'@!\@ENSC_HAVE_CXX_COMPILER_TRUE@ !g; \
			s!@'ENSC_HAVE_CXX_COMPILER_FALSE'@!\@ENSC_HAVE_CXX_COMPILER_FALSE@ !g; \
			s!@'HAVE_PYTHON_TRUE'@!\@HAVE_PYTHON_TRUE@ !g; \
			s!@'HAVE_PYTHON_FALSE'@!\@HAVE_PYTHON_FALSE@ !g; \
			s!@'PYEXECDIR'@!$(pyexecdir)!g; \
			$(ENSC_PATHPROG_SED)

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

pkgcachedir =		$(localstatedir)/cache/vservers
pkgstatedir =		$(localstatedir)/run/vservers
pkgstaterevdir =	$(confdefaultsdir)/run.rev
vserverpkgdir =		$(vserverdir)/.pkg
vshelperstatedir =	$(localstatedir)/run/vshelper

fix_SCRPTS		=  $(addprefix $(pkglibdir)/, $(notdir $(pkglib_SCRIPTS))) \
			   $(addprefix $(legacydir)/, $(notdir $(legacy_SCRIPTS))) \
			   $(addprefix $(sbindir)/,   $(notdir $(sbin_SCRIPTS))) \
			   $(addprefix $(initrddir)/, $(notdir $(initrd_SCRIPTS)))

update-doc:		doc
			$(top_srcdir)/update-doc

install-data-hook:	install-fix-script-paths install-create-dirs

install-create-dirs:
			$(mkinstalldirs) -m 0755 $(DESTDIR)$(pkgstatedir) $(DESTDIR)$(pkgstatedir).rev $(DESTDIR)$(vshelperstatedir)
			$(mkinstalldirs) -m 0700 $(DESTDIR)$(pkgcachedir)
			$(mkinstalldirs) -m 0755 $(DESTDIR)$(confdefaultsdir)
			f=$(DESTDIR)$(confdefaultsdir)/vdirbase;  test -e "$$f" || ln -sf '$(vserverdir)'      "$$f"
			f=$(DESTDIR)$(confdefaultsdir)/cachebase; test -e "$$f" || ln -sf '$(pkgcachedir)'     "$$f"
			f=$(DESTDIR)$(pkgstaterevdir);            test -e "$$f" || ln -sf '$(pkgstatedir).rev' "$$f"

install-fix-script-paths:
			test "/usr/lib/util-vserver" = "$(pkglibdir)" || \
			for i in $(fix_SCRPTS); do \
				f="$(DESTDIR)$$i"; \
				$(SED) -e 's!/usr/lib/util-vserver!$(pkglibdir)!g' "$$f" >"$$f.tmp"; \
				$(CMP) -s "$$f.tmp" "$$f" || cat "$$f.tmp" >"$$f"; \
				rm -f "$$f.tmp"; \
			done

install-distribution:	_install-vserverdir _install-vshelperlink

_install-vshelperlink:
			$(mkinstalldirs) $(DESTDIR)/sbin
			ln -sf ..$(pkglibdir)/vshelper $(DESTDIR)/sbin/vshelper

_install-vserverdir:
			$(mkinstalldirs) $(DESTDIR)$(vserverdir) $(DESTDIR)$(vserverpkgdir)


.fixups:		config.status util-vserver.spec
			chmod a-w util-vserver.spec
			touch $@

if USE_DIETLIBC
$(addsuffix $(EXEEXT), $(DIETPROGS)) \
$(addsuffix $(OBJEXT), $(DIETPROGS)) \
		:		CC=$(DIET_CC)
else	!USE_DIETLIBC
nodist_lib_dietlibc_not_enabled_error_a_SOURCES = \
			lib-dietlibc-not-enabled-error.c
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
include $(top_srcdir)/ensc_fmt/Makefile-files
include $(top_srcdir)/lib_internal/Makefile-files
#include $(top_srcdir)/vserver-start/Makefile-files
include $(top_srcdir)/gentoo/Makefile-files
if HAVE_PYTHON
include $(top_srcdir)/python/Makefile-files
endif
include $(top_srcdir)/debian/Makefile-files

include $(top_srcdir)/m4/gpgsig.am
include $(top_srcdir)/m4/validate.am
include $(top_srcdir)/m4/changelog.am
include $(top_srcdir)/m4/pkgconfig.am
include $(top_srcdir)/m4/libsel.am
include $(top_srcdir)/m4/install-notify.am

.PHONY:		FORCE update-doc
