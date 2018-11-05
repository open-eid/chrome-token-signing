Chrome Token Signing [1.0.8](https://github.com/open-eid/chrome-token-signing/releases/tag/v1.0.8) release notes
--------------------------------------------
- Add hardware token filter on Windows (#114)
- Sign macOS policy profile (#112)
- Handle PKCS11 CKR_PIN_LOCKED error code (#115)
- Add IDEMIA driver paths

[Full Changelog](https://github.com/open-eid/chrome-token-signing/compare/v1.0.7...v1.0.8)

Chrome Token Signing [1.0.7](https://github.com/open-eid/chrome-token-signing/releases/tag/v1.0.7) release notes
--------------------------------------------
- Use OpenSC driver with unkown ATR on OSX/Linux (#75)
- Add new Finland and Lithuanian driver paths (#81)
- Use SCardGetStatusChange to get ATR-s and avoid to connecting with card (#85)
- Filter attributes with token filter (#87)
- Code and build improvements

[Full Changelog](https://github.com/open-eid/chrome-token-signing/compare/v1.0.6...v1.0.7)


Chrome Token Signing [1.0.6](https://github.com/open-eid/chrome-token-signing/releases/tag/v1.0.6) release notes
--------------------------------------------
- Add ECDSA token support
- Code and build improvements

[Full Changelog](https://github.com/open-eid/chrome-token-signing/compare/v1.0.5...v1.0.6)


Chrome Token Signing [1.0.5](https://github.com/open-eid/chrome-token-signing/releases/tag/v1.0.5) release notes
--------------------------------------------
- Lithuania token iprovements
- Code and build improvements

[Full Changelog](https://github.com/open-eid/chrome-token-signing/compare/v1.0.4...v1.0.5)


Chrome Token Signing [1.0.4](https://github.com/open-eid/chrome-token-signing/releases/tag/v1.0.4) release notes
--------------------------------------------
- Firefox 50 support
- Build improvements

[Full Changelog](https://github.com/open-eid/chrome-token-signing/compare/v1.0.3...v1.0.4)



Chrome Token Signing 1.0.3 release notes
--------------------------------------------
Changes compared to ver 1.0.2

- Fixed RUS PIN attempts remaining text not fitting into dialog window on Linux
- Fixed duplicate certificates appearing in certificate selection dialog with Finnish eID and technical error on sign
- Added Token Support for Lithuanian eID on OSX and Linux
- Added Lithuanian Token support when using PKCS#11 on windows native component.
- Added Token Support for Latvian and Finnish eID on OSX and Linux.

Notes: 
- You need to have middleware for the eID that you are using installed (Finland, Estonia, Latvia or Lithuania). No other configuration should be necessary.
- Note that if you want to use PKCS#11 in windows, then you must add the extension manually from the release zip file, enable forcing pkcs11 and add the path to pkcs#11 module you want to use in the extension options menu. See https://github.com/open-eid/chrome-token-signing/wiki/DeveloperTips on how to manually add extension to chrome


Chrome Token Signing 1.0.2 release notes
--------------------------------------------
Changes compared to ver 1.0.0

- Added support for pkcs#11 on Windows
	- Added support for negative scenarios with pkcs#11 backend in Windows (wrong pin, no pin, retries, etc.)
	- Windows native host can be configured to load a specific pkcs#11 module by sending {pkcs11ModulePath:"full-path-to-pkcs11-module"} e.g. {pkcs11ModulePath:"C:\opensc-pkcs11.dll"} from extension. In order to use this {forcePkcs11:true} must also be sent.
	- The native host looks for pkcs#11 module with name opensc-pkcs11.dll when forcePkcs11:true is sent without pkcs11ModulePath
- Removed openssl dependency on Windows build
- Fixed CAPI support for Windows
	- Reversing CAPI signature
	- Fixed error checking when using CAPI
	- Fixed Latvian ID-card signing with CAPI (in addition to PKCS#11)
	
List of known issues: https://github.com/open-eid/chrome-token-signing/wiki/Known-Issues
List of supported tokens: https://github.com/open-eid/chrome-token-signing/wiki/Token-Support


Chrome Token Signing 1.0.0 release notes
--------------------------------------------
Changes compared to ver 3.9

- Changed native messaging API https://github.com/open-eid/chrome-token-signing/wiki/NativeMessagingAPI
- Added native messaging hosts for OSX and Windows
- Added support for CAPI and CNG on Windows and PKCS#11 on Linux/OSX
- Changed linux native messaging host UI from using GTK to QT
- Native messaging hosts changed to being stateful
	- to maintain certificate selection binding (only user-confirmed certificate can be used for signing) 
	- to check for same origin in all messages after first technically valid CERT request containing origin
- Added python test suite for native messaging hosts
- Improved logging
- changed error codes to use string constants instead of numbers
- Changed extension API 
- Moved extension to Chrome Web Store
- Added branding to extension
- Changed versioning - starting from 1.0.0


Release 3.9 Chrome Linux plugin release notes
--------------------------------------------

- First release instead of NPAPI for Linux







