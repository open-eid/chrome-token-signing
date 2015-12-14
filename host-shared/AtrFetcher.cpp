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

#include "AtrFetcher.h"
#include "Logger.h"
#include "BinaryUtils.h"
#include <stdexcept>

class PcscException : public std::runtime_error {
private:
    std::string message;
public:
    PcscException(const std::string &msg) :runtime_error(msg), message(msg){}
    std::string getErrorMessage() const{
        return message;
    }
};

std::vector<std::string> AtrFetcher::fetchAtr() {
    try {
        populate();
    }
    catch (const PcscException &e) {
        atrs.resize(0);
    }
    release();
    return atrs;
}

void AtrFetcher::populate() {
    establishContext();
    listReaders();
    populateAtrs();
}

void AtrFetcher::establishContext() {
    LONG err = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (err != SCARD_S_SUCCESS) {
        _log("SCardEstablishContext ERROR: %x", err);
        throw PcscException("establish_context");
    }
}

void AtrFetcher::listReaders() {
    DWORD size;
    LONG err = SCardListReaders(hContext, NULL, NULL, &size);
    if( err != SCARD_S_SUCCESS || !size ) {
        _log("SCardListReaders || !size ERROR: %x", err);
        throw PcscException("list_readers");
    }
    std::vector<char> readers(size, sizeof(char));
    
    err = SCardListReaders(hContext, NULL, readers.data(), &size);
    if( err != SCARD_S_SUCCESS) {
        _log("SCardListReaders ERROR: %x", err);
        throw PcscException("list_readers");
    }
    readers.resize(size, sizeof(char));

    for(std::vector<char>::const_iterator i = readers.begin(); i != readers.end(); ++i) {
        std::string readerName(&*i);
        if( !readerName.empty() ) {
            CardReader *reader = new CardReader(readerName, hContext);
            _log("found reader: %s", reader->getName().c_str());
            readerList.push_back(reader);
        }
        i += readerName.size();
    }
}

void AtrFetcher::populateAtrs() {
    for (int i = 0; i<readerList.size(); i++) {
        CardReader *reader = readerList[i];
        try {
            reader -> connect();
            reader -> populateAtr();
            atrs.push_back(reader -> getAtr());
        } catch (PcscException &e) {
            _log("reader %s threw exception: %s", reader -> getName().c_str(), e.getErrorMessage().c_str());
        }
    }
}

void AtrFetcher::release() {
    for (int i = 0; i<readerList.size(); i++) {
        readerList[i] -> release();
    }
    SCardReleaseContext(hContext);
}

void CardReader::connect() {
    LONG err = SCardConnect(context, name.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &cardHandle, &activeProtocol);
    if( err != SCARD_S_SUCCESS) {
        _log("SCardConnect ERROR for %s: %x", name.c_str(), err);
        throw PcscException("connect");
    }
}

void CardReader::populateAtr() {
    std::vector<char> name(100, sizeof(char));
    std::vector<unsigned char> bAtr(MAX_ATR_SIZE, 0);
    DWORD nameSize = name.size();
    DWORD atrSize = bAtr.size();
    DWORD dwState, dwProtocol;
    
    LONG err = SCardStatus(cardHandle, name.data(), &nameSize, &dwState, &dwProtocol, bAtr.data(), &atrSize);
    if( err != SCARD_S_SUCCESS) {
        _log("SCardStatus ERROR for %s: %x", this -> name.c_str(), err);
        throw PcscException("status");
    }
    bAtr.resize(atrSize);
    atr = BinaryUtils::bin2hex(bAtr);
    _log("Set ATR = %s for reader %s", atr.c_str(), this -> name.c_str());
}

std::string CardReader::getAtr() {
    return atr;
}

std::string CardReader::getName() {
    return name;
}

void CardReader::release() {
    SCardDisconnect(cardHandle, SCARD_LEAVE_CARD);
}
