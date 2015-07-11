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

# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.

!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF
!include VERSION.mk
SIGN = signtool sign /v /a /s MY /n "RIIGI INFOSUSTEEMI AMET" /fd SHA256 /du http://installer.id.ee /t http://timestamp.verisign.com/scripts/timstamp.dll

build:
	msbuild /p:Configuration=Release;Platform=Win32 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=$(RELEASE_VERSION) /property:BUILD_NUMBER=$(BUILD_NUMBER) host-windows\host-windows.sln

pkg:
	$(SIGN) host-windows/Release/chrome-token-signing.exe
	"$(WIX)\bin\candle.exe" host-windows\chrome-token-signing.wxs -dVERSION=$(VERSIONEX)
	"$(WIX)\bin\light.exe" -out chrome-token-signing_$(VERSIONEX).msi chrome-token-signing.wixobj -v -ext WixUIExtension -dWixUILicenseRtf=LICENSE.LGPL.rtf -dWixUIDialogBmp=host-windows/dlgbmp.bmp
	$(SIGN) chrome-token-signing_$(VERSIONEX).msi

pkg-unsigned:
	"$(WIX)\bin\candle.exe" host-windows\chrome-token-signing.wxs -dVERSION=$(VERSIONEX)
	"$(WIX)\bin\light.exe" -out chrome-token-signing_$(VERSIONEX).msi chrome-token-signing.wixobj -v -ext WixUIExtension -dWixUILicenseRtf=LICENSE.LGPL.rtf -dWixUIDialogBmp=host-windows/dlgbmp.bmp
	
test: build
	python host-test\pipe-test.py -v
