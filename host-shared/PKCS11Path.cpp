/*
 * Chrome Token Signing Native Host
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "PKCS11Path.h"
#include "BinaryUtils.h"
#include "Logger.h"

#ifdef _WIN32
#undef UNICODE
#include <Shlobj.h>
#include <Knownfolders.h>
#include <winscard.h>
#elif defined __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#endif

#include <map>

#define MAX_ATR_SIZE 33	/**< Maximum ATR size */

std::vector<std::string> PKCS11Path::atrList() {
	SCARDCONTEXT hContext;
	std::vector<std::string> result;
	LONG err = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
	if (err != SCARD_S_SUCCESS) {
		_log("SCardEstablishContext ERROR: %x", err);
		return result;
	}

	DWORD size;
	err = SCardListReaders(hContext, NULL, NULL, &size);
	if (err != SCARD_S_SUCCESS || !size) {
		_log("SCardListReaders || !size ERROR: %x", err);
		SCardReleaseContext(hContext);
		return result;
	}

	std::string readers(size, 0);
	err = SCardListReaders(hContext, NULL, &readers[0], &size);
	readers.resize(size);
	if (err != SCARD_S_SUCCESS) {
		_log("SCardListReaders ERROR: %x", err);
		SCardReleaseContext(hContext);
		return result;
	}

	for (std::string::const_iterator i = readers.begin(); i != readers.end(); ++i) {
		std::string name(&*i);
		i += name.size();
		if (name.empty())
			continue;

		_log("found reader: %s", name.c_str());

		SCARDHANDLE cardHandle = 0;
		DWORD dwProtocol = 0;
		LONG err = SCardConnect(hContext, name.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &cardHandle, &dwProtocol);
		if (err != SCARD_S_SUCCESS) {
			_log("SCardConnect ERROR for %s: %x", name.c_str(), err);
			continue;
		}

		std::vector<unsigned char> bAtr(MAX_ATR_SIZE, 0);
		DWORD atrSize = DWORD(bAtr.size());
		err = SCardStatus(cardHandle, nullptr, nullptr, nullptr, nullptr, bAtr.data(), &atrSize);
		if (err == SCARD_S_SUCCESS) {
			bAtr.resize(atrSize);
			std::string atr = BinaryUtils::bin2hex(bAtr);
			result.push_back(atr);
			_log("Set ATR = %s for reader %s", atr.c_str(), name.c_str());
		}
		else {
			_log("SCardStatus ERROR for %s: %x", name.c_str(), err);
		}
		SCardDisconnect(cardHandle, SCARD_LEAVE_CARD);
	}

	SCardReleaseContext(hContext);
	return result;
}

PKCS11Path::Params PKCS11Path::getPkcs11ModulePath() {
#ifdef __APPLE__
    static const std::string estPath("/Library/OpenSC/lib/opensc-pkcs11.so");
    static const std::string latPath("/Library/latvia-eid/lib/otlv-pkcs11.so");
    static const std::string finPath("/Library/mPolluxDigiSign/libcryptoki.dylib");
    static const std::string litPath("/Library/Security/tokend/CCSuite.tokend/Contents/Frameworks/libccpkip11.dylib");
    static const std::string eTokenPath("/Library/Frameworks/eToken.framework/Versions/Current/libeToken.dylib");
#elif defined _WIN32
    // Use PKCS11 driver on windows to avoid PIN buffering
    static const std::string litPath = [] {
        wchar_t *programFilesX86 = 0;
        SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, NULL, &programFilesX86);
        int size = WideCharToMultiByte(CP_UTF8, 0, programFilesX86, wcslen(programFilesX86), NULL, 0, NULL, NULL);
        std::string path(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, programFilesX86, wcslen(programFilesX86), &path[0], size, NULL, NULL);
        CoTaskMemFree(programFilesX86);
        return path + "\\CryptoTech\\CryptoCard\\CCPkiP11.dll";
    }();
#else
    static const std::string estPath("opensc-pkcs11.so");
    static const std::string latPath("otlv-pkcs11.so");
    static const std::string finPath("opensc-pkcs11.so");
    static const std::string litPath("/usr/lib/ccs/libccpkip11.so");
    static const std::string eTokenPath("/usr/local/lib/libeTPkcs11.dylib");
#endif
    static const std::map<std::string, Params> m = {
#ifdef _WIN32
        {"3BFD1800008031FE4553434536302D43443134352D46CD", {"C:\\Windows\\System32\\aetpkss1.dll", "PIN", "PIN"}},
#else
        {"3BFE9400FF80B1FA451F034573744549442076657220312E3043", {estPath, "PIN1", "PIN2"}},
        {"3B6E00FF4573744549442076657220312E30", {estPath, "PIN1", "PIN2"}},
        {"3BDE18FFC080B1FE451F034573744549442076657220312E302B", {estPath, "PIN1", "PIN2"}},
        {"3B5E11FF4573744549442076657220312E30", {estPath, "PIN1", "PIN2"}},
        {"3B6E00004573744549442076657220312E30", {estPath, "PIN1", "PIN2"}},

        {"3BFE1800008031FE454573744549442076657220312E30A8", {estPath, "PIN1", "PIN2"}},
        {"3BFE1800008031FE45803180664090A4561B168301900086", {estPath, "PIN1", "PIN2"}},
        {"3BFE1800008031FE45803180664090A4162A0083019000E1", {estPath, "PIN1", "PIN2"}},
        {"3BFE1800008031FE45803180664090A4162A00830F9000EF", {estPath, "PIN1", "PIN2"}},

        {"3BF9180000C00A31FE4553462D3443432D303181", {estPath, "PIN1", "PIN2"}},
        {"3BF81300008131FE454A434F5076323431B7", {estPath, "PIN1", "PIN2"}},
        {"3BFA1800008031FE45FE654944202F20504B4903", {estPath, "PIN1", "PIN2"}},
        {"3BFE1800008031FE45803180664090A4162A00830F9000EF", {estPath, "PIN1", "PIN2"}},

        {"3BDD18008131FE45904C41545649412D65494490008C", {latPath, "PIN1", "PIN2"}},

        {"3B7B940000806212515646696E454944", {finPath, "PIN1", "PIN2"}},

        {"3BD5180081313A7D8073C8211030", {eTokenPath, "PIN", "PIN"}},
        {"3BD518008131FE7D8073C82110F4", {eTokenPath, "PIN", "PIN"}},
#endif
        {"3BF81300008131FE45536D617274417070F8", {litPath, "PIN", "PIN"}},
        {"3B7D94000080318065B08311C0A983009000", {litPath, "PIN", "PIN"}},
        {"3B7D94000080318065B0831100C883009000", {litPath, "PIN", "PIN"}},
        {"3B9F9681B1FE451F070064051EB20031B0739621DB00900050", {litPath, "PIN", "PIN"}},
        {"3B9F90801FC30068104405014649534531C800000000", {litPath, "PIN", "PIN"}},
    };

    for (const std::string &atr : atrList()) {
        auto it = m.find(atr);
        if (it != m.end())
            return it->second;
	}
	return Params();
}
