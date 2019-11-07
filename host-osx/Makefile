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

BUILD_NUMBER ?= 0
include ../VERSION.mk
SIGNER ?= Riigi Infosüsteemi Amet
PATH_EXTERNAL = /Library/Application\ Support/Google/Chrome/External\ Extensions
PATH_NATIVE = /Library/Google/Chrome/NativeMessagingHosts
PATH_FIREFOX = /Library/Application\ Support/Mozilla/NativeMessagingHosts
PATH_PREFERENCES = /Library/Preferences
TMP_ROOT = $(PWD)/tmp
TMP_CHROME = $(PWD)/tmp_chrome
TMP_FIREFOX = $(PWD)/tmp_firefox
TMP_POLICY = $(PWD)/tmp_policy
TMP_POLICY_SIGNED = $(PWD)/tmp_signed
PKG_BUILD = pkgbuild --version $(VERSION) --install-location /
PKG_BUILD_SIGNED = $(PKG_BUILD) --sign "Developer ID Installer: $(SIGNER)"
PKG_CHROME = token-signing-chrome
PKG_FIREFOX = token-signing-firefox
PKG_POLICY = token-signing-chrome-policy
PROJ = xcodebuild -project chrome-token-signing.xcodeproj -configuration Release MARKETING_VERSION=$(VERSION) CURRENT_PROJECT_VERSION=$(BUILD_NUMBER)
TARGET = $(TMP_ROOT)/Library/Google/Chrome/NativeMessagingHosts/chrome-token-signing.app

$(TARGET): *.mm
	$(PROJ) install DSTROOT=$(TMP_ROOT)

codesign: $(TARGET)
	codesign -f --entitlements chrome-token-signing.entitlements --options=runtime -s "Developer ID Application: $(SIGNER)" $<

clean:
	$(PROJ) clean
	git clean -dfx

$(TMP_CHROME): $(TARGET)
	mkdir -p $@/$(PATH_NATIVE) $@/$(PATH_EXTERNAL)
	cp -R $< $@/$(PATH_NATIVE)
	cp -v ee.ria.esteid.json $@/$(PATH_NATIVE)
	cp -v ../ckjefchnfjhjfedoccjbhjpbncimppeg.json $@/$(PATH_EXTERNAL)

$(TMP_FIREFOX): $(TARGET)
	mkdir -p $@/$(PATH_NATIVE) $@/$(PATH_FIREFOX)
	cp -R $< $@/$(PATH_NATIVE)
	cp -v ee.ria.esteid.firefox.json $@/$(PATH_FIREFOX)/ee.ria.esteid.json

$(TMP_POLICY):
	mkdir -p $@

$(PKG_CHROME): $(TMP_CHROME)
	$(PKG_BUILD) --root $< --identifier ee.ria.$@ $@.pkg

$(PKG_FIREFOX): $(TMP_FIREFOX)
	$(PKG_BUILD) --root $< --identifier ee.ria.$@ --scripts scripts_firefox $@.pkg

$(PKG_POLICY): $(TMP_POLICY)
	$(PKG_BUILD) --root $< --identifier ee.ria.$@ --scripts scripts_policy $@.pkg

pkg: $(PKG_CHROME) $(PKG_FIREFOX) $(PKG_POLICY)

signed: codesign $(TMP_CHROME) $(TMP_FIREFOX) $(TMP_POLICY)
	$(PKG_BUILD_SIGNED) --root $(TMP_CHROME) --identifier ee.ria.$(PKG_CHROME) $(PKG_CHROME).pkg
	$(PKG_BUILD_SIGNED) --root $(TMP_FIREFOX) --identifier ee.ria.$(PKG_FIREFOX) --scripts scripts_firefox $(PKG_FIREFOX).pkg
	mkdir -p $(TMP_POLICY_SIGNED)
	cp scripts_policy/postinstall $(TMP_POLICY_SIGNED)
	security cms -S -N "Developer ID Application: $(SIGNER)" -i scripts_policy/force-install.plist -o $(TMP_POLICY_SIGNED)/force-install.plist
	$(PKG_BUILD_SIGNED) --root $(TMP_POLICY) --identifier ee.ria.$(PKG_POLICY) --scripts $(TMP_POLICY_SIGNED) $(PKG_POLICY).pkg

# helpers

install:
	$(PROJ)
	mkdir -p $(PATH_NATIVE) $(PATH_FIREFOX) $(PATH_EXTERNAL)
	cp -rv build/Release/chrome-token-signing.app $(PATH_NATIVE)
	cp -v ee.ria.esteid.json $(PATH_NATIVE)
	cp -v ee.ria.esteid.firefox.json $(PATH_FIREFOX)/ee.ria.esteid.json
	cp -v ../ckjefchnfjhjfedoccjbhjpbncimppeg.json $(PATH_EXTERNAL)
	cp -v org.mozilla.firefox.plist $(PATH_PREFERENCES)/

uninstall:
	rm -f $(PATH_EXTERNAL)/ckjefchnfjhjfedoccjbhjpbncimppeg.json
	rm -f $(PATH_NATIVE)/ee.ria.esteid.json
	rm -f $(PATH_FIREFOX)/ee.ria.esteid.json
	rm -rf $(PATH_NATIVE)/chrome-token-signing.app
	rm -f $(PATH_PREFERENCES)/org.mozilla.firefox.plist

test:
	python ../host-test/stateless-test.py $(TARGET)/Contents/MacOS/chrome-token-signing
