EXTERNAL_PATH = /Library/Application\ Support/Google/Chrome/External\ Extensions
NATIVE_PATH = /Library/Google/Chrome/NativeMessagingHosts

all: *.mm
	xcodebuild -project chrome-token-signing.xcodeproj
	
clean:
	xcodebuild -project chrome-token-signing.xcodeproj clean

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
