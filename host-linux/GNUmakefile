#
# Chrome Token Signing Native Host
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

ifeq ($(BUILD_NUMBER),)
BUILD_NUMBER = 0
endif
include ../VERSION.mk
include /etc/lsb-release

all:
	qmake -qt=5 VERSION=$(VERSIONEX)
	make -f Makefile

clean:
	make -f Makefile distclean

install:
	make -f Makefile install

uninstall:
	make -f Makefile uninstall

debian:
	cd ..; \
	git-archive-all ../chrome-token-signing_$(VERSIONEX).orig.tar.gz; \
	dch --distribution unstable -v $(VERSIONEX)-$(subst .,,$(DISTRIB_RELEASE)) "Release: $(VERSIONEX)."; \
	dpkg-buildpackage

