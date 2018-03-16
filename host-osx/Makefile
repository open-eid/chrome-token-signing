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
FIREFOX_PATH = /Library/Application\ Support/Mozilla/NativeMessagingHosts
FFEXTENSION_PATH = /Library/Application\ Support/Mozilla/Extensions/\{ec8030f7-c20a-464f-9b0e-13a3a9e97384\}

BUILD_NUMBER ?= 0
include ../VERSION.mk

SIGNER ?= Riigi Infosüsteemi Amet
TMPROOT = $(PWD)/tmp
TARGET = $(TMPROOT)/Library/Google/Chrome/NativeMessagingHosts/chrome-token-signing.app
PKG = chrome-token-signing.pkg

PROJ = xcodebuild -project chrome-token-signing.xcodeproj -configuration Release VERSION=$(VERSION) BUILD_NUMBER=$(BUILD_NUMBER)

$(TARGET): *.mm
	$(PROJ) install DSTROOT=$(TMPROOT)

codesign: $(TARGET)
	codesign -f --entitlements chrome-token-signing.entitlements -s "Developer ID Application: $(SIGNER)" "$(TARGET)"

clean:
	$(PROJ) clean
	git clean -dfx

$(PKG): $(TARGET)
	pkgbuild --version $(VERSION) \
		--root $(TMPROOT) \
		--identifier ee.ria.chrome-token-signing \
		--install-location / \
		--scripts scripts \
		$(PKG)

pkg: $(PKG)

signed: codesign
	pkgbuild --version $(VERSION) \
		--root $(TMPROOT) \
		--identifier ee.ria.chrome-token-signing \
		--install-location / \
		--scripts scripts \
		--sign "Developer ID Installer: $(SIGNER)" \
		$(PKG)


# helpers

install:
	mkdir -p $(NATIVE_PATH) $(FIREFOX_PATH) $(FFEXTENSION_PATH) $(EXTERNAL_PATH)
	cp -rv build/Release/chrome-token-signing.app $(NATIVE_PATH)
	cp -v ee.ria.esteid.json $(NATIVE_PATH)
	cp -v ee.ria.esteid.firefox.json $(FIREFOX_PATH)/ee.ria.esteid.json
	cp -v ../{443830f0-1fff-4f9a-aa1e-444bafbc7319}.xpi $(FFEXTENSION_PATH)
	cp -v ../ckjefchnfjhjfedoccjbhjpbncimppeg.json $(EXTERNAL_PATH)

uninstall:
	rm -f $(EXTERNAL_PATH)/ckjefchnfjhjfedoccjbhjpbncimppeg.json
	rm -f $(NATIVE_PATH)/ee.ria.esteid.json
	rm -f $(FIREFOX_PATH)/ee.ria.esteid.json
	rm -f $(FFEXTENSION_PATH)/{443830f0-1fff-4f9a-aa1e-444bafbc7319}.xpi
	rm -rf $(NATIVE_PATH)/chrome-token-signing.app

test:
	python ../host-test/stateless-test.py $(TARGET)/Contents/MacOS/chrome-token-signing
