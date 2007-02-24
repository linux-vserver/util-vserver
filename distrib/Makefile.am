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

defaults_DATA =		misc/debootstrap.uri \
			misc/fstab \
			misc/mtab \
			misc/vprocunhide-files \
			misc/vunify-exclude \
			misc/environment \
			misc/context.start

nobase_distrib_SCRIPTS	=  redhat/initpost \
			   redhat/initpre \
			   redhat/rc.sysinit \
			   gentoo/initpost \
			   gentoo/initpre

nobase_distrib_DATA	=  defaults/devs \
			   defaults/apt.conf \
			   defaults/rpm/macros \
			   defaults/apt/apt.conf \
			   template/initpost template/initpre \
			   rh9/apt/rpmpriorities    rh9/pkgs/01    rh9/pkgs/02 \
			   fc1/apt/rpmpriorities    fc1/pkgs/01    fc1/pkgs/02 \
			   fc2/apt/rpmpriorities    fc2/pkgs/01    fc2/pkgs/02 \
			   fc3/apt/rpmpriorities    fc3/pkgs/01    fc3/pkgs/02    fc3/pkgs/03 \
			   fc3/yum/yum.conf $(wildcard fc3/yum.repos.d/*.repo) \
			   $(wildcard fc3/rpmlist.d/*.lst) $(wildcard fc3/rpmlist.d/*.opt) \
			   fc4/apt/rpmpriorities    fc4/pkgs/01    fc4/pkgs/02    fc4/pkgs/03 \
			   fc4/yum/yum.conf $(wildcard fc4/yum.repos.d/*.repo) \
			   $(wildcard fc4/rpmlist.d/*.lst) $(wildcard fc4/rpmlist.d/*.opt) \
			   fc5/apt/rpmpriorities    fc5/pkgs/01    fc5/pkgs/02    fc5/pkgs/03 \
			   fc5/yum/yum.conf $(wildcard fc5/yum.repos.d/*.repo) \
			   $(wildcard fc5/rpmlist.d/*.lst) $(wildcard fc5/rpmlist.d/*.opt) \
			   fc6/apt/rpmpriorities    fc6/pkgs/01    fc6/pkgs/02    fc6/pkgs/03 \
			   fc6/yum/yum.conf $(wildcard fc6/yum.repos.d/*.repo) \
			   $(wildcard fc6/rpmlist.d/*.lst) $(wildcard fc6/rpmlist.d/*.opt) \
			   centos4/pkgs/01 centos4/pkgs/02 centos4/pkgs/03 \
			   centos4/yum/yum.conf $(wildcard centos4/yum.repos.d/*.repo) \
			   suse91/apt/rpmpriorities suse91/pkgs/01 suse91/pkgs/02 \
			   suse91/rpm/macros \
			   gentoo/init-vserver.sh gentoo/net.vserver

nobase_confdistrib_DATA	=  rh9/apt/sources.list \
			   fc1/apt/sources.list \
			   fc2/apt/sources.list \
			   fc3/apt/sources.list \
			   fc4/apt/sources.list \
			   fc5/apt/sources.list \
			   fc6/apt/sources.list \
			   suse91/apt/sources.list

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT \
			=  $(nobase_distrib_SCRIPTS)

EXTRA_DIST		=  $(nobase_distrib_DATA) \
			   $(nobase_distrib_SCRIPTS) \
			   $(nobase_confdistrib_DATA) \
			   $(defaults_DATA)

redhat_style		=  rh9 fc1 fc2 fc3 fc4 fc5 fc6 centos4

install-exec-hook:	install-notify-xtra
install-data-hook:	install-data-xtras

install-data-xtras:
			l="$(redhat_style)"; for i in $$l; do \
				ln -sf ../redhat/initpost $(DESTDIR)$(distribdir)/$$i/ && \
				ln -sf ../redhat/initpre  $(DESTDIR)$(distribdir)/$$i/; \
			done
			$(mkinstalldirs) $(DESTDIR)$(confdir) \
				$(DESTDIR)$(confdistribdir)/.common/pubkeys \
				$(DESTDIR)$(confdefaultsdir) \
				$(DESTDIR)$(confdefaultsdir)/apps/vunify/hash \
				$(DESTDIR)$(confdefaultsdir)/files


include $(top_srcdir)/m4/install-notify.am
