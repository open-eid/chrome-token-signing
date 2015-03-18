# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.

include version.properties

!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION).$(RELEASE_VERSION).$(BUILD_NUMBER)

build:
	msbuild /p:Configuration=Release;Platform=Win32 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=$(RELEASE_VERSION) /property:BUILD_NUMBER=$(BUILD_NUMBER) host-windows\host-windows.sln

pkg:
	"$(WIX)\bin\candle.exe" host-windows\chrome-token-signing.wxs -dVERSION=$(VERSION)
	"$(WIX)\bin\light.exe" -out chrome-token-signing_$(VERSION).msi chrome-token-signing.wixobj -v -ext WixUIExtension -dWixUILicenseRtf=LICENSE.LGPL.rtf -dWixUIDialogBmp=host-windows/dlgbmp.bmp

test: build
	python host-test\pipe-test.py -v
