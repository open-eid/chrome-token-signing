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
#include <Shlwapi.h>
#include <Knownfolders.h>
#include <winscard.h>
#elif defined __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#include <unistd.h>
#else
#include <winscard.h>
#include <unistd.h>
#endif

#include <codecvt>
#include <cstring>
#include <map>

std::vector<std::string> PKCS11Path::atrList() {
    SCARDCONTEXT hContext = 0;
    std::vector<std::string> result;
    LONG err = SCardEstablishContext(SCARD_SCOPE_USER, nullptr, nullptr, &hContext);
    if (err != SCARD_S_SUCCESS) {
        _log("SCardEstablishContext ERROR: %x", (unsigned int)err);
        return result;
    }

    DWORD size = 0;
    err = SCardListReaders(hContext, nullptr, nullptr, &size);
    if (err != SCARD_S_SUCCESS || !size) {
        _log("SCardListReaders || !size ERROR: %x", (unsigned int)err);
        SCardReleaseContext(hContext);
        return result;
    }

    std::string readers(size, 0);
    err = SCardListReaders(hContext, nullptr, &readers[0], &size);
    readers.resize(size);
    if (err != SCARD_S_SUCCESS) {
        _log("SCardListReaders ERROR: %x", (unsigned int)err);
        SCardReleaseContext(hContext);
        return result;
    }

    std::vector<SCARD_READERSTATE> list;
    for (const char *name = readers.c_str(); *name; name += strlen(name) + 1) {
        _log("found reader: %s", name);
        list.push_back({ name, nullptr, 0, 0, 0, {} });
    }

    err = SCardGetStatusChange(hContext, 0, list.data(), DWORD(list.size()));
    if (err != SCARD_S_SUCCESS)
        _log("SCardGetStatusChange ERROR: %x", (unsigned int)err);
    for(const SCARD_READERSTATE &state: list)
    {
        if (state.dwEventState & SCARD_STATE_PRESENT)
        {
            std::string atr = BinaryUtils::bin2hex(state.rgbAtr, state.cbAtr);
            result.push_back(atr);
            _log("Set ATR = %s for reader %s", atr.c_str(), state.szReader);
        }
    }

    SCardReleaseContext(hContext);
    return result;
}

PKCS11Path::Params PKCS11Path::getPkcs11ModulePath() {
#ifdef _WIN32
    // Use PKCS11 driver on windows to avoid PIN buffering
    static const std::string litPath = [] {
        PWSTR programFilesX86 = 0;
        SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, nullptr, &programFilesX86);
        std::wstring path = programFilesX86;
        CoTaskMemFree(programFilesX86);
        if (PathFileExistsW((path + L"\\PWPW\\pwpw-card-pkcs11.dll").c_str()))
            path += L"\\PWPW\\pwpw-card-pkcs11.dll";
        else
            path += L"\\CryptoTech\\CryptoCard\\CCPkiP11.dll";
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(path);
    }();
    static const std::string akisPath("C:\\Windows\\System32\\akisp11.dll");
#elif defined __APPLE__
    static const std::string openscPath("/Library/OpenSC/lib/opensc-pkcs11.so");
    static const std::string estPath = openscPath;
    static const std::string latPath("/Library/latvia-eid/lib/eidlv-pkcs11.bundle/Contents/MacOS/eidlv-pkcs11");
    static const std::string finPath("/Library/mPolluxDigiSign/libcryptoki.dylib");
    static const std::string lit1Path("/Library/Security/tokend/CCSuite.tokend/Contents/Frameworks/libccpkip11.dylib");
    static const std::string lit2Path("/Library/PWPW-Card/pwpw-card-pkcs11.so");
    static const std::string litPath = access(lit1Path.c_str(), F_OK) == 0 ? lit1Path : lit2Path;
    static const std::string belPath("/usr/local/lib/beid-pkcs11.bundle/Contents/MacOS/libbeidpkcs11.dylib");
    static const std::string eTokenPath("/Library/Frameworks/eToken.framework/Versions/Current/libeToken.dylib");
    static const std::string IDPrimePath("/Library/Frameworks/eToken.framework/Versions/Current/libIDPrimePKCS11.dylib");
    static const std::string acsPath("/usr/local/lib/libacos5pkcs11.dylib");
    static const std::string akisPath("/usr/local/lib/libakisp11.dylib");
#else
    static const std::string openscPath("opensc-pkcs11.so");
    static const std::string estPath = openscPath;
    static const std::string latPath("/opt/latvia-eid/lib/eidlv-pkcs11.so");
    static const std::string lit1Path("/usr/lib/ccs/libccpkip11.so");
#ifdef __LP64__
    static const std::string finPath("/usr/lib64/libcryptoki.so");
    static const std::string lit2Path("/usr/lib64/pwpw-card-pkcs11.so");
#else
    static const std::string lit2Path("pwpw-card-pkcs11.so");
    static const std::string finPath("libcryptoki.so");
#endif
    static const std::string litPath = access(lit1Path.c_str(), F_OK) == 0 ? lit1Path : lit2Path;
    static const std::string belPath("libbeidpkcs11.so.0");
    static const std::string eTokenPath("/usr/lib/libeTPkcs11.so");
    static const std::string IDPrimePath("/usr/lib/libIDPrimePKCS11.so");
    static const std::string acsPath("/lib/libacospkcs11.so");
    static const std::string akisPath("/usr/lib/libpkcs11wrapper.so");
#endif
    static const std::map<std::string, Params> m = {
#ifdef _WIN32
        {"3BFD1800008031FE4553434536302D43443134352D46CD", {"C:\\Windows\\System32\\aetpkss1.dll", "PIN", "PIN"}},
#else
        {"3BFE1800008031FE454573744549442076657220312E30A8", {estPath, "PIN1", "PIN2"}}, //ESTEID_V3_COLD_DEV1_ATR
        {"3BFE1800008031FE45803180664090A4561B168301900086", {estPath, "PIN1", "PIN2"}}, //ESTEID_V3_WARM_DEV1_ATR
        {"3BFE1800008031FE45803180664090A4162A0083019000E1", {estPath, "PIN1", "PIN2"}}, //ESTEID_V3_WARM_DEV2_ATR
        {"3BFE1800008031FE45803180664090A4162A00830F9000EF", {estPath, "PIN1", "PIN2"}}, //ESTEID_V3_WARM_DEV3_ATR
        {"3BFA1800008031FE45FE654944202F20504B4903", {estPath, "PIN1", "PIN2"}}, //ESTEID_V3.5_COLD_ATR
        {"3BDB960080B1FE451F830012233F536549440F9000F1", {estPath, "PIN1", "PIN2"}}, // IDEMIA 2018

        {"3BDD18008131FE45904C41545649412D65494490008C", {latPath, "PIN1", "PIN2"}},
        {"3BDB960080B1FE451F830012428F536549440F900020", {latPath, "PIN1", "PIN2"}},

        {"3B7B940000806212515646696E454944", {finPath, "PIN1", "PIN2"}},
        {"3B7F9600008031B865B0850300EF1200F6829000", {finPath, "PIN1", "PIN2"}},

        {"3B9813400AA503010101AD1311", {belPath, "PIN", "PIN"}},

        {"3BD5180081313A7D8073C8211030", {eTokenPath, "PIN", "PIN"}},
        {"3BD518008131FE7D8073C82110F4", {eTokenPath, "PIN", "PIN"}},
        {"3BFF9600008131804380318065B0850300EF120FFE82900066", {IDPrimePath, "PIN", "PIN"}},
        {"3BBE9600004105200000000000000000009000", {acsPath, "PIN", "PIN" }},
#endif
        {"3BF81300008131FE45536D617274417070F8", {litPath, "PIN", "PIN"}},
        {"3B7D94000080318065B08311C0A983009000", {litPath, "PIN", "PIN"}},
        {"3B7D94000080318065B0831100C883009000", {litPath, "PIN", "PIN"}},
        {"3B9F9681B1FE451F070064051EB20031B0739621DB00900050", {litPath, "PIN", "PIN"}},
        {"3B9F90801FC30068104405014649534531C800000000", {litPath, "PIN", "PIN"}},
        {"3BBA11008131FE4D55454B41452056312E30AE", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451112318073B3A180E9", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451212318073B3A180EA", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451213318073B3A180EB", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451252318073B3A180AA", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451253318073B3A180AB", {akisPath, "PIN", "PIN"}},
        {"3B9F158131FE45806755454B41451221318073B3A1805A", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451221318073B3A180D9", {akisPath, "PIN", "PIN"}},
        {"3B9F138131FE45806755454B41451221318073B3A1805C", {akisPath, "PIN", "PIN"}},
        {"3B9F138131FE45806755454B41451261318073B3A1801C", {akisPath, "PIN", "PIN"}},
        {"3B9F158131FE45806755454B41451261318073B3A1801A", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451261318073B3A18099", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE458065544320201231C073F621808105B3", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451292318073B3A1806A", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451293318073B3A1806B", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B41451312318073B3A180EB", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B414512A4318073B3A1805C", {akisPath, "PIN", "PIN"}},
        {"3B9F968131FE45806755454B414512A5318073B3A1805D", {akisPath, "PIN", "PIN"}},
        {"3B9F978131FE458065544312210031C073F62180810593", {akisPath, "PIN", "PIN"}}
    };

    const std::vector<std::string> list = atrList();
    for (const std::string &atr : list) {
        const auto it = m.find(atr);
        if (it != m.cend())
            return it->second;
    }
#ifdef _WIN32
    return {};
#else
    if (!list.empty())
        _log("Unknown ATR '%s' using default module '%s'", list[0].c_str(), openscPath.c_str());
    return {openscPath, "PIN", "PIN"};
#endif
}
