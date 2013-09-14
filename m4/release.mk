# Copyright (C) 2012 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 and/or 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

D = $(abspath .)
NAME = util-vserver
VERSION = 0.30.215
DIST_ARCHIVES = $(NAME)-$(VERSION).tar.gz

BRANCH = master
REFTAG = release-$(VERSION)

COMPAT_REFERENCE = b30eee53f811b1fa1a4a21f84466ab79c9f422c0
COMPAT_NUMBER = 2939

GIT = git
SED = sed

unexport

_d = $(abspath $(D))
_cnt = $(shell cd $(_d) && $(GIT) rev-list $(REFTAG)..$(BRANCH) | wc -l)
_rev = $(shell cd $(_d) && $(GIT) rev-parse --verify --short $(BRANCH))
_full_rev = $(shell cd $(_d) && $(GIT) rev-parse --verify $(BRANCH))
_compat_cnt = $(shell cd $(_d) && $(GIT) rev-list $(COMPAT_REFERENCE)..$(BRANCH) | wc -l)

rel:
	t=`mktemp -d /tmp/util-vserver.XXXXXX` && trap "rm -rf $$t" EXIT && \
	$(GIT) clone -b $(BRANCH) $(D) $$t/$(notdir $(D)) && \
	$(MAKE) -C $$t/$(notdir $(D)) .rel MAKEFILES='$(abspath $(MAKEFILE_LIST))' \
		CNT='$(_cnt)' REV='$(_rev)' FULL_REV=$(_full_rev) \
		COMPAT_CNT="$$(( $(_compat_cnt) + $(COMPAT_NUMBER) ))" \
		DISTDIR='$(abspath .)'

NEW_VERSION = $(VERSION).$(CNT)+g$(REV)
NEW_ARCHIVES = $(patsubst $(NAME)-$(VERSION).%,$(NAME)-$(NEW_VERSION).%,$(DIST_ARCHIVES))

SED_CMD = \
  -e "s!$(VERSION)'!$(NEW_VERSION)'!"

.news.devel:	NEWS
	rm -f $@
	( \
		echo 'version $(NEW_VERSION)' | sed 'p;s!.!=!g' && \
		echo &&\
		echo -e '\t- git snapshot $(FULL_REV)' && \
		echo -e '\t  (corresponds to SVN number $(COMPAT_CNT))' && \
		echo && \
		cat $< \
	) > $@

.rel:	.news.devel
	rm -f NEWS
	mv .news.devel NEWS
	autoreconf -i -f

	touch -r configure .stamp.configure
	$(SED) -i $(SED_CMD) configure
	touch -r .stamp.configure configure

	./configure
	+env -u MAKEFILES -u MAKEFLAGS $(MAKE) dist

	install -p -m 0644 $(NEW_ARCHIVES) $(DISTDIR)/
