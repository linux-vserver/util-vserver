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
distribdir		=  $(pkglibdir)/distributions
confdistribdir		=  $(confdir)/.distributions

nobase_distrib_DATA	=  defaults/devs \
			   defaults/rpm/macros \
			   rh9/apt/rpmpriorities \
			   rh9.0.93/apt/rpmpriorities \
			   rh9/pkgs/01 rh9/pkgs/02 \
			   rh9.0.93/pkgs/01 rh9.0.93/pkgs/02

nobase_confdistrib_DATA	=  rh9/apt/sources.list \
			   rh9.0.93/apt/sources.list

EXTRA_DIST		=  $(nobase_distrib_DATA) \
			   $(nobase_confdistrib_DATA)
