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

DIETPROGS =		$(ENSC_VECTOR_LIBS)

BUILT_SOURCES =		linuxcaps.h pathconfig.h linuxvirtual.h

man_MANS =

kernel_HDRS =		kernel/context.h \
			kernel/inode.h \
			kernel/legacy.h \
			kernel/limit.h \
			kernel/namespace.h \
			kernel/network.h \
			kernel/sched.h \
			kernel/signal.h \
			kernel/switch.h

EXTRA_PROGRAMS =
EXTRA_DIST =		pathconfig.h.pathsubst \
			util-vserver.spec \
			THANKS \
			$(kernel_HDRS)

check_PROGRAMS =
check_SCRIPTS =
TESTS =

TESTS_ENVIRONMENT =

AM_CPPFLAGS		=  -I $(top_srcdir)/lib -I $(top_srcdir)/ensc_wrappers -D _GNU_SOURCE $(RELEASE_CPPFLAGS)

CLEANFILES =		$(BUILT_SOURCES) \
			.*.pathsubst.stamp */.*.pathsubst.stamp */*/.*.pathsubst.stamp \
			linuxcaps.h pathconfig.h

DISTCLEANFILES =	FEATURES.txt


AM_INSTALLCHECK_STD_OPTIONS_EXEMPT =

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

pathsubst_RULES =	s!@'PKGLIBDIR'@!$(pkglibdir)!g; \
			s!@'LEGACYDIR'@!$(legacydir)!g; \
			s!@'SBINDIR'@!$(sbindir)!g; \
			s!@'CONFDIR'@!$(confdir)!g; \
			s!@'SYSCONFDIR'@!$(sysconfdir)!g; \
			s!@'PKGSTATEDIR'@!$(pkgstatedir)!g; \
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
			s!@'AWK'@!$(AWK)!g; \
			s!@'GREP'@!$(GREP)!g; \
			s!@'IP'@!$(IP)!g; \
			s!@'IPTABLES'@!$(IPTABLES)!g; \
			s!@'MODPROBE'@!$(MODPROBE)!g; \
			s!@'MOUNT'@!$(MOUNT)!g; \
			s!@'NAMEIF'@!$(NAMEIF)!g; \
			s!@'NICE'@!$(NICE)!g; \
			s!@'PS'@!$(PS)!g; \
			s!@'RMMOD'@!$(RMMOD)!g; \
			s!@'TAR'@!$(TAR)!g; \
			s!@'UMOUNT'@!$(UMOUNT)!g; \
			s!@'VCONFIG'@!$(VCONFIG)!g; \
			s!@'WC'@!$(WC)!g; \
			s!@'WGET'@!$(WGET)!g; \
			s!@'XSLTPROC'@!$(XSLTPROC)!g; \
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

pkgstatedir =		$(localstatedir)/run/vservers
pkgstaterevdir =	$(pkgstatedir).rev
vserverpkgdir =		$(vserverdir)/.pkg
vshelperstatedir =	$(localstatedir)/run/vshelper

fix_SCRPTS		=  $(addprefix $(pkglibdir)/, $(notdir $(pkglib_SCRIPTS))) \
			   $(addprefix $(legacydir)/, $(notdir $(legacy_SCRIPTS))) \
			   $(addprefix $(sbindir)/,   $(notdir $(sbin_SCRIPTS))) \
			   $(addprefix $(initrddir)/, $(notdir $(initrd_SCRIPTS)))

install-data-hook:	install-fix-script-paths install-create-dirs

install-create-dirs:
			$(mkinstalldirs) -m 755 $(DESTDIR)$(pkgstatedir) $(DESTDIR)$(pkgstaterevdir) $(DESTDIR)$(vshelperstatedir)
			$(mkinstalldirs) -m 755 $(DESTDIR)$(confdefaultsdir)
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

install-distribution:	_install-vserverdir _install-vshelperlink

_install-vshelperlink:
			$(mkinstalldirs) $(DESTDIR)/sbin
			ln -sf ..$(pkglibdir)/vshelper $(DESTDIR)/sbin/vshelper

_install-vserverdir:
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
