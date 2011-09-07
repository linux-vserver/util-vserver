## $Id$

## Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
##  
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##  
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##  
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

confdir =		$(sysconfdir)/vservers
defaultsdir =		$(pkglibdir)/defaults
distribdir =		$(pkglibdir)/distributions
confdistribdir =	$(confdir)/.distributions
confdefaultsdir =	$(confdir)/.defaults

defaults_DATA =		misc/debootstrap.mirror \
			misc/debootstrap.uri \
			misc/fstab \
			misc/mtab \
			misc/vprocunhide-files \
			misc/vunify-exclude \
			misc/environment \
			misc/context.start \
			misc/h2ext.desc

nobase_distrib_SCRIPTS	=  redhat/initpost \
			   redhat/initpre \
			   redhat/rc.sysinit \
			   redhat/initctl \
			   gentoo/initpost \
			   gentoo/initpre \
			   debian/initpost \
			   alpine/initpre \
			   alpine/initpost

nobase_distrib_DATA	=  defaults/devs \
			   defaults/apt.conf \
			   defaults/rpm/macros \
			   defaults/apt/apt.conf \
			   template/initpost template/initpre \
			   rh9/apt/rpmpriorities    rh9/pkgs/01    rh9/pkgs/02 \
			   fc1/apt/rpmpriorities    fc1/pkgs/01    fc1/pkgs/02 \
			   fc2/apt/rpmpriorities    fc2/pkgs/01    fc2/pkgs/02 \
			   fc3/apt/rpmpriorities    fc3/pkgs/01    fc3/pkgs/02    fc3/pkgs/03 \
			   fc3/yum/yum.conf $(wildcard $(srcdir)/fc3/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/fc3/rpmlist.d/*.lst) $(wildcard $(srcdir)/fc3/rpmlist.d/*.opt) \
			   fc4/apt/rpmpriorities    fc4/pkgs/01    fc4/pkgs/02    fc4/pkgs/03 \
			   fc4/yum/yum.conf $(wildcard $(srcdir)/fc4/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/fc4/rpmlist.d/*.lst) $(wildcard $(srcdir)/fc4/rpmlist.d/*.opt) \
			   fc5/apt/rpmpriorities    fc5/pkgs/01    fc5/pkgs/02    fc5/pkgs/03 \
			   fc5/yum/yum.conf $(wildcard $(srcdir)/fc5/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/fc5/rpmlist.d/*.lst) $(wildcard $(srcdir)/fc5/rpmlist.d/*.opt) \
			   fc6/apt/rpmpriorities    fc6/pkgs/01    fc6/pkgs/02    fc6/pkgs/03 \
			   fc6/yum/yum.conf $(wildcard $(srcdir)/fc6/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/fc6/rpmlist.d/*.lst) $(wildcard $(srcdir)/fc6/rpmlist.d/*.opt) \
			   f7/apt/rpmpriorities    f7/pkgs/01    f7/pkgs/02    f7/pkgs/03 \
			   f7/yum/yum.conf $(wildcard $(srcdir)/f7/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/f7/rpmlist.d/*.lst) $(wildcard $(srcdir)/f7/rpmlist.d/*.opt) \
			   f8/apt/rpmpriorities    f8/pkgs/01    f8/pkgs/02    f8/pkgs/03 \
			   f8/yum/yum.conf $(wildcard $(srcdir)/f8/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/f8/rpmlist.d/*.lst) $(wildcard $(srcdir)/f8/rpmlist.d/*.opt) \
			   $(wildcard $(srcdir)/f8/pubkeys/RPM-GPG*) \
			   f9/pkgs/01 f9/pkgs/02 f9/pkgs/03 f9/yum/yum.conf \
			   $(wildcard $(srcdir)/f9/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/f9/rpmlist.d/*.lst) $(wildcard $(srcdir)/f9/rpmlist.d/*.opt) \
			   $(wildcard $(srcdir)/f9/pubkeys/RPM-GPG*) \
			   f10/pkgs/01 f10/pkgs/02 f10/pkgs/03 f10/yum/yum.conf \
			   $(wildcard $(srcdir)/f10/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/f10/rpmlist.d/*.lst) $(wildcard $(srcdir)/f10/rpmlist.d/*.opt) \
			   f11/pkgs/01 f11/pkgs/02 f11/pkgs/03 f11/yum/yum.conf \
			   $(wildcard $(srcdir)/f11/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/f11/rpmlist.d/*.lst) $(wildcard $(srcdir)/f11/rpmlist.d/*.opt) \
			   f12/pkgs/01 f12/pkgs/02 f12/pkgs/03 f12/yum/yum.conf \
			   $(wildcard $(srcdir)/f12/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/f12/rpmlist.d/*.lst) $(wildcard $(srcdir)/f12/rpmlist.d/*.opt) \
			   f13/pkgs/01 f13/pkgs/02 f13/pkgs/03 f13/yum/yum.conf \
			   $(wildcard $(srcdir)/f13/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/f13/rpmlist.d/*.lst) $(wildcard $(srcdir)/f13/rpmlist.d/*.opt) \
			   f14/pkgs/01 f14/pkgs/02 f14/pkgs/03 f14/yum/yum.conf \
			   $(wildcard $(srcdir)/f14/yum.repos.d/*.repo) \
			   $(wildcard $(srcdir)/f14/rpmlist.d/*.lst) $(wildcard $(srcdir)/f14/rpmlist.d/*.opt) \
			   centos4/pkgs/01 centos4/pkgs/02 centos4/pkgs/03 \
			   centos4/yum/yum.conf $(wildcard $(srcdir)/centos4/yum.repos.d/*.repo) \
			   centos5/pkgs/01 centos5/pkgs/02 centos5/pkgs/03 \
			   centos5/yum/yum.conf $(wildcard $(srcdir)/centos5/yum.repos.d/*.repo) \
			   sl6/pkgs/01 sl6/pkgs/02 sl6/pkgs/03 \
			   sl6/yum/yum.conf $(wildcard $(srcdir)/sl6/yum.repos.d/*.repo) \
			   suse91/apt/rpmpriorities suse91/pkgs/01 suse91/pkgs/02 \
			   suse91/rpm/macros \
			   gentoo/init-vserver.sh gentoo/net.vserver \
			   gentoo/reboot.sh gentoo/shutdown.sh

nobase_confdistrib_DATA	=  rh9/apt/sources.list \
			   fc1/apt/sources.list \
			   fc2/apt/sources.list \
			   fc3/apt/sources.list \
			   fc4/apt/sources.list \
			   fc5/apt/sources.list \
			   fc6/apt/sources.list \
			   f7/apt/sources.list \
			   suse91/apt/sources.list

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT \
			=  $(nobase_distrib_SCRIPTS)

EXTRA_DIST		=  $(nobase_distrib_DATA) \
			   $(nobase_distrib_SCRIPTS) \
			   $(nobase_confdistrib_DATA) \
			   $(defaults_DATA)

redhat_style		=  rh9 fc1 fc2 fc3 fc4 fc5 fc6 f7 f8 f9 f10 f11 f12 f13 f14 centos4 centos5 sl6
sid_style		=  etch lenny squeeze wheezy sid

install-exec-hook:	install-notify-xtra
install-data-hook:	install-data-xtras

install-data-xtras:
			l="$(redhat_style)"; for i in $$l; do \
				ln -sf ../redhat/initpost $(DESTDIR)$(distribdir)/$$i/ && \
				ln -sf ../redhat/initpre  $(DESTDIR)$(distribdir)/$$i/; \
			done
			l="$(sid_style)"; for i in $$l; do \
				ln -sf debian $(DESTDIR)$(distribdir)/$$i; \
			done
			$(mkinstalldirs) $(DESTDIR)$(confdir) \
				$(DESTDIR)$(confdistribdir)/.common/pubkeys \
				$(DESTDIR)$(confdefaultsdir) \
				$(DESTDIR)$(confdefaultsdir)/apps/vunify/hash \
				$(DESTDIR)$(confdefaultsdir)/files


include $(top_srcdir)/m4/install-notify.am
