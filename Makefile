# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.
!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF
VERSION=1.0.0.$(BUILD_NUMBER)

build:
	msbuild /p:Configuration=Release;Platform=Win32 /property:BUILD_NUMBER=$(BUILD_NUMBER) host-windows\host-windows.sln

pkg:
	"$(WIX)\bin\candle.exe" host-windows\chrome-token-signing.wxs -dVERSION=$(VERSION)
	"$(WIX)\bin\light.exe" -out chrome-token-signing_$(VERSION).msi chrome-token-signing.wixobj -v -ext WixUIExtension

test: build
	python host-test\pipe-test.py -v
