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
TMP_CHROME = $(PWD)/tmp_chrome
TMP_FIREFOX = $(PWD)/tmp_firefox
POLICYROOT = $(PWD)/tmp_policy
POLICYSIGNED = $(PWD)/tmpsigned
TARGET = $(TMPROOT)/Library/Google/Chrome/NativeMessagingHosts/chrome-token-signing.app
PKG_CHROME = token-signing-chrome.pkg
PKG_FIREFOX = token-signing-firefox.pkg
POLICY = token-signing-chrome-policy.pkg

PROJ = xcodebuild -project chrome-token-signing.xcodeproj -configuration Release VERSION=$(VERSION) BUILD_NUMBER=$(BUILD_NUMBER)
PKG_BUILD = pkgbuild --version $(VERSION) --install-location / 
PKG_BUILD_SIGNED = $(PKG_BUILD) --sign "Developer ID Installer: $(SIGNER)"

$(TARGET): *.mm
	$(PROJ) install DSTROOT=$(TMPROOT)
	$(MAKE) separate

codesign: $(TARGET) 
	codesign -f --entitlements chrome-token-signing.entitlements --options=runtime -s "Developer ID Application: $(SIGNER)" $<

clean:
	$(PROJ) clean
	git clean -dfx

$(PKG_CHROME): $(TARGET)
	$(PKG_BUILD) --root $(TMP_CHROME) --identifier ee.ria.token-signing-chrome $@

$(PKG_FIREFOX): $(TARGET)
	$(PKG_BUILD) --root $(TMP_FIREFOX) --identifier ee.ria.token-signing-firefox $@

$(POLICY):
	mkdir -p $(POLICYROOT)
	$(PKG_BUILD) --root $(POLICYROOT) --identifier ee.ria.token-signing-chrome-policy --scripts scripts $@

pkg: $(PKG_CHROME) $(PKG_FIREFOX) $(POLICY)

signed: codesign
	$(PKG_BUILD_SIGNED) --root $(TMP_CHROME) --identifier ee.ria.token-signing-chrome $(PKG_CHROME)
	$(PKG_BUILD_SIGNED) --root $(TMP_FIREFOX) --identifier ee.ria.token-signing-firefox $(PKG_FIREFOX)
	mkdir -p $(POLICYROOT) $(POLICYSIGNED)
	cp scripts/postinstall $(POLICYSIGNED)
	security cms -S -N "Developer ID Application: $(SIGNER)" -i scripts/force-install.plist -o $(POLICYSIGNED)/force-install.plist
	$(PKG_BUILD_SIGNED) --root $(POLICYROOT) --identifier ee.ria.token-signing-chrome-policy --scripts $(POLICYSIGNED) $(POLICY)

# helpers

separate:
	mkdir -p $(TMP_CHROME)/$(NATIVE_PATH) $(TMP_CHROME)/$(EXTERNAL_PATH)
	cp -R $(TARGET) $(TMP_CHROME)/$(NATIVE_PATH)
	cp -v ee.ria.esteid.json $(TMP_CHROME)/$(NATIVE_PATH)
	cp -v ../ckjefchnfjhjfedoccjbhjpbncimppeg.json $(TMP_CHROME)/$(EXTERNAL_PATH)
	mkdir -p $(TMP_FIREFOX)/$(NATIVE_PATH) $(TMP_FIREFOX)/$(FIREFOX_PATH) $(TMP_FIREFOX)/$(FFEXTENSION_PATH)
	cp -R $(TARGET) $(TMP_FIREFOX)/$(NATIVE_PATH)
	cp -v ee.ria.esteid.firefox.json $(TMP_FIREFOX)/$(FIREFOX_PATH)/ee.ria.esteid.json
	cp -v ../{443830f0-1fff-4f9a-aa1e-444bafbc7319}.xpi $(TMP_FIREFOX)/$(FFEXTENSION_PATH)

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
