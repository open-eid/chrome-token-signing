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

BUILD_NUMBER ?= 0
include ../VERSION.mk

SIGNER ?= Riigi Infosüsteemi Amet
OPENSSL ?= $(PWD)/target
TMPROOT = $(PWD)/tmp
TARGET = $(TMPROOT)/Library/Google/Chrome/NativeMessagingHosts/chrome-token-signing.app
PKG = chrome-token-signing.pkg

PROJ = xcodebuild -project chrome-token-signing.xcodeproj -configuration Release VERSION=$(VERSION) BUILD_NUMBER=$(BUILD_NUMBER) OPENSSL=$(OPENSSL)

$(TARGET): *.mm $(OPENSSL)
	$(PROJ) install DSTROOT=$(TMPROOT)

codesign: $(TARGET)
	codesign -f --entitlements chrome-token-signing.entitlements -s "$(SIGNER)" "$(TARGET)"

clean:
	$(PROJ) clean
	git clean -dfx

$(PKG): $(TARGET)
	pkgbuild --version $(VERSION) \
		--root $(TMPROOT) \
		--identifier ee.ria.chrome-token-signing \
                --install-location / \
		$(PKG)

pkg: $(PKG)

signed: codesign
	pkgbuild --version $(VERSION) \
		--root $(TMPROOT) \
		--identifier ee.ria.chrome-token-signing \
                --install-location / \
                --sign "$(SIGNER)" \
		$(PKG)



$(OPENSSL):
	test -e openssl || git clone --depth=1 https://github.com/openssl/openssl.git -b OpenSSL_1_0_2-stable
	(cd openssl \
	&& KERNEL_BITS=64 ./config --prefix=$(OPENSSL) -mmacosx-version-min=10.9 no-shared no-ssl2 no-idea no-dtls no-psk no-srp no-apps \
	&& make depend \
	&& make \
	&& make install_sw)

# helpers

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
	python ../host-test/stateless-test.py $(TARGET)/Contents/MacOS/chrome-token-signing
