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

EXTERNAL_PATH = /Library/Application\ Support/Google/Chrome/External\ Extensions
NATIVE_PATH = /Library/Google/Chrome/NativeMessagingHosts
ifeq ($(BUILD_NUMBER),)
BUILD_NUMBER = 0
endif
include ../VERSION.mk
SIGNCERT = Developer ID Application: Riigi Infosüsteemi Amet
INSTCERT = Developer ID Installer: Riigi Infosüsteemi Amet
PROJ = xcodebuild -project chrome-token-signing.xcodeproj VERSION=$(VERSION) BUILD_NUMBER=$(BUILD_NUMBER) $(PARAMS)

all: *.mm
	$(PROJ)

clean:
	$(PROJ) clean

pkg:
	$(PROJ) DSTROOT=/tmp/chrome-token-signing.dst install
	codesign -f -s "$(SIGNCERT)" "/tmp/chrome-token-signing.dst/$(NATIVE_PATH)/chrome-token-signing.app"
	pkgbuild --root /tmp/chrome-token-signing.dst --sign "$(INSTCERT)" chrome-token-signing.pkg
	productbuild --distribution chrome-token-signing.dist.xml --sign "$(INSTCERT)" \
		--package-path . --resources . chrome-token-signing-dist_$(VERSIONEX).pkg
	mv chrome-token-signing.pkg chrome-token-signing_$(VERSIONEX).pkg
	pkgbuild --component build/Release/chrome-token-signing.app.dSYM --sign "$(INSTCERT)" \
		--identifier "ee.ria.chrome-token-signing-dbg" --version "$(VERSION)" \
		--install-location $(NATIVE_PATH)/chrome-token-signing.app/Contents/MacOS \
		chrome-token-signing-dbg_$(VERSIONEX).pkg

install:
	mkdir -p $(NATIVE_PATH)
	cp -rv build/Release/chrome-token-signing.app $(NATIVE_PATH)
	cp -v ee.ria.esteid.json $(NATIVE_PATH)
	mkdir -p $(EXTERNAL_PATH)
	cp -v ../ckjefchnfjhjfedoccjbhjpbncimppeg.json $(EXTERNAL_PATH)

uninstall:
	rm -f $(EXTERNAL_PATH)/ckjefchnfjhjfedoccjbhjpbncimppeg.json
	rm -f $(NATIVE_PATH)/ee.ria.esteid.json
	rm -rf $(NATIVE_PATH)/chrome-token-signing.app

test:
	python ../host-test/stateless-test.py build/Release/chrome-token-signing.app/Contents/MacOS/chrome-token-signing
