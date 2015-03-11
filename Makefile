VERSION=1.0.0

build:
	msbuild host-windows\host-windows.sln

pkg:
	"$(WIX)\bin\candle.exe" host-windows\chrome-token-signing.wxs -dVERSION=$(VERSION)
	"$(WIX)\bin\light.exe" -out chrome-token-signing.msi chrome-token-signing.wixobj -v -ext WixUIExtension

test: build
	python host-test\pipe-test.py -v
