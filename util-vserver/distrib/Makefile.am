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

confdir			=  $(sysconfdir)/vservers
defaultsdir		=  $(pkglibdir)/defaults
distribdir		=  $(pkglibdir)/distributions
confdistribdir		=  $(confdir)/.distributions
confdefaultsdir		=  $(confdir)/.defaults
confdefaultsinitdir	=  $(confdefaultsdir)/init

defaults_DATA =		misc/debootstrap.uri \
			misc/fstab \
			misc/mtab \
			misc/vunify-exclude

nobase_distrib_SCRIPTS	=  redhat/initpost

nobase_distrib_DATA	=  defaults/devs \
			   defaults/apt.conf \
			   defaults/rpm/macros \
			   defaults/apt/apt.conf \
			   rh9/apt/rpmpriorities rh9/pkgs/01 rh9/pkgs/02 \
			   fc1/apt/rpmpriorities fc1/pkgs/01 fc1/pkgs/02

nobase_confdistrib_DATA	=  rh9/apt/sources.list \
			   fc1/apt/sources.list

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT \
			=  $(nobase_distrib_SCRIPTS)

EXTRA_DIST		=  $(nobase_distrib_DATA) \
			   $(nobase_distrib_SCRIPTS) \
			   $(nobase_confdistrib_DATA) \
			   $(defaults_DATA)

redhat_style		=  rh9 fc1


install-data-hook:
			l="$(redhat_style)"; for i in $$l; do \
				ln -sf ../redhat/initpost $(DESTDIR)$(distribdir)/$$i/; \
			done
			$(mkinstalldirs) $(DESTDIR)$(confdir) $(DESTDIR)$(confdefaultsdir) \
				$(DESTDIR)$(confdefaultsinitdir)
