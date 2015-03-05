EXTERNAL_PATH = /Library/Application\ Support/Google/Chrome/External\ Extensions
NATIVE_PATH = /Library/Google/Chrome/NativeMessagingHosts
VERSION = 1.0.0
BUILD_NUMBER = 1
SIGNCERT = Developer ID Application: Riigi Infosüsteemi Amet
INSTCERT = Developer ID Installer: Riigi Infosüsteemi Amet
PROJ = xcodebuild -project chrome-token-signing.xcodeproj VERSION=$(VERSION) BUILD_NUMBER=$(BUILD_NUMBER)

all: *.mm
	$(PROJ)

clean:
	$(PROJ) clean

install:
	$(PROJ) DSTROOT=/ install

pkg:
	$(PROJ) DSTROOT=/tmp/chrome-token-signing.dst install
	codesign -f -s "$(SIGNCERT)" "/tmp/chrome-token-signing.dst/$(NATIVE_PATH)/chrome-token-signing.app"
	pkgbuild --root /tmp/chrome-token-signing.dst --sign "$(INSTCERT)" chrome-token-signing_$(VERSION).$(BUILD_NUMBER).pkg

uninstall:
	rm -f $(EXTERNAL_PATH)/ckjefchnfjhjfedoccjbhjpbncimppeg.json
	rm -f $(NATIVE_PATH)/ee.ria.esteid.json
	rm -rf $(NATIVE_PATH)/chrome-token-signing.app

test:
	python ../host-test/stateless-test.py build/Release/chrome-token-signing.app/Contents/MacOS/chrome-token-signing
