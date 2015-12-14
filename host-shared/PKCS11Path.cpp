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

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include "PKCS11Path.h"
#include "Logger.h"
#include "BinaryUtils.h"
#include "AtrFetcher.h"

static std::map<std::string, std::string> createMap() {
#ifdef __APPLE__
    const std::string estPath("/Library/EstonianIDCard/lib/esteid-pkcs11.so");
    const std::string latPath("/Library/latvia-eid/lib/otlv-pkcs11.so");
    const std::string finPath("/Library/mPolluxDigiSign/libcryptoki.dylib");
    const std::string litPath("/System/Library/Security/tokend/CCSuite.tokend/Contents/Frameworks/libccpkip11.dylib");
#else
    const std::string estPath("opensc-pkcs11.so");
    const std::string latPath("otlv-pkcs11.so");
    const std::string finPath("opensc-pkcs11.so");
    const std::string litPath("/usr/lib/ccs/libccpkip11.so");
#endif
    std::map<std::string, std::string> m;
    
    m["3BFE9400FF80B1FA451F034573744549442076657220312E3043"] = estPath;
    m["3B6E00FF4573744549442076657220312E30"] = estPath;
    m["3BDE18FFC080B1FE451F034573744549442076657220312E302B"] = estPath;
    m["3B5E11FF4573744549442076657220312E30"] = estPath;
    m["3B6E00004573744549442076657220312E30"] = estPath;
    
    m["3BFE1800008031FE454573744549442076657220312E30A8"] = estPath;
    m["3BFE1800008031FE45803180664090A4561B168301900086"] = estPath;
    m["3BFE1800008031FE45803180664090A4162A0083019000E1"] = estPath;
    m["3BFE1800008031FE45803180664090A4162A00830F9000EF"] = estPath;
    
    m["3BF9180000C00A31FE4553462D3443432D303181"] = estPath;
    m["3BF81300008131FE454A434F5076323431B7"] = estPath;
    m["3BFA1800008031FE45FE654944202F20504B4903"] = estPath;
    m["3BFE1800008031FE45803180664090A4162A00830F9000EF"] = estPath;
    
    m["3BDD18008131FE45904C41545649412D65494490008C"] = latPath;
    
    m["3B7B940000806212515646696E454944"] = finPath;
    
    m["3BF81300008131FE45536D617274417070F8"] = litPath;
    m["3B7D94000080318065B08311C0A983009000"] = litPath;
    return m;
}
const std::map<std::string, std::string> atrToDriverMap = createMap();

std::string PKCS11Path::getPkcs11ModulePath() {
    std::vector<std::string> atrs = AtrFetcher().fetchAtr();
    
    if (atrs.empty()) {
        _log("unable to determine card atr... ");
        return "";
    }
    std::string driver = "";
    for (int i = 0; i < atrs.size(); i++) {
        auto it = atrToDriverMap.find(atrs[i]);
        if (it == atrToDriverMap.end()) {
            continue;
        }
        driver = it -> second; //TODO: don't deal with multiple different drivers at the moment, just get the first match
        break;
    }
    if (driver.empty()) {
        _log("no suitable driver found...");
        return "";
    }
    _log("selected pkcs module = %s", driver.c_str());
    return driver;
}
