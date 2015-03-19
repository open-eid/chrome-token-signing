# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.

!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF
!include VERSION.mk
SIGN = signtool sign /v /a /ac "C:/codesigning/MSCV-VSClass3.cer" /n "RIIGI INFOSUSTEEMI AMET" /fd SHA256 /du http://installer.id.ee /t http://timestamp.verisign.com/scripts/timstamp.dll

build:
	msbuild /p:Configuration=Release;Platform=Win32 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=$(RELEASE_VERSION) /property:BUILD_NUMBER=$(BUILD_NUMBER) host-windows\host-windows.sln

pkg:
	$(SIGN) host-windows/Release/host-windows.exe
	"$(WIX)\bin\candle.exe" host-windows\chrome-token-signing.wxs -dVERSION=$(VERSIONEX)
	"$(WIX)\bin\light.exe" -out chrome-token-signing_$(VERSIONEX).msi chrome-token-signing.wixobj -v -ext WixUIExtension -dWixUILicenseRtf=LICENSE.LGPL.rtf -dWixUIDialogBmp=host-windows/dlgbmp.bmp
	$(SIGN) chrome-token-signing_$(VERSIONEX).msi

test: build
	python host-test\pipe-test.py -v
