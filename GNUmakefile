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

# This is the Makefile for OSX/Linux. See Makefile for Windows NMake.
UNAME = `uname`
RELEASE = `grep '"version"' extension/manifest.json  | cut -d'"' -f 4`

detect:
	make $(UNAME)

Linux:
	make -C host-linux

Darwin:
	make -C host-osx

# Make the zip to be uploaded to chrome web store
release:
	test ! -f extension-$(RELEASE).zip
	test -z "`git status -s extension`"
	git clean -dfx extension
	zip -r extension-$(RELEASE).zip extension

test: detect
	# wildcard will resolve to an empty string with a missing file
	# so that OSX will not run with xvfb
	$(wildcard /usr/bin/xvfb-run) python host-test/pipe-test.py -v

# Make the targzip for the native components
# FIXME: git describe vs $(RELEASE) ?
dist:
	git-archive-all chrome-token-signing-`git describe --tags --always`.tar.gz
