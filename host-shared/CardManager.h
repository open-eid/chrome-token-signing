/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#pragma once

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA384_LENGTH 48
#define BINARY_SHA512_LENGTH 64

#include "PinString.h"

#include <vector>

class CardManager {
 public:
    virtual ~CardManager() {}
    virtual std::vector<unsigned int> getAvailableTokens() {return std::vector<unsigned int>();}
    virtual bool isReaderPresent() {return getAvailableTokens().size() > 0;}
    virtual bool isCardInReader() const {return false;}
    CardManager *getManagerForReader(int /*readerId*/) {return new CardManager();}
    virtual std::vector<unsigned char> sign(const std::vector<unsigned char> &/*hash*/, const PinString &/*pin*/){return std::vector<unsigned char>();}
    virtual std::string getCardName(){return "Mari-Liis Mannik";}
    virtual std::string getPersonalCode(){return "47101010033";}
    virtual int getPIN2RetryCount() const {return 3;}
    virtual std::vector<unsigned char> getSignCert() const {return std::vector<unsigned char>();}
    virtual std::string getCN() const {return "";}
    virtual std::string getType() const {return "";}
    virtual time_t getValidTo() const {return 0;}
    virtual time_t getValidFrom() const {return 0;}
    std::string getKeyUsage() const {return "Non-Repudiation";}
    virtual std::string getIssuerCN() const {return "";}
    virtual std::string getCertSerialNumber() const {return "";}
    virtual bool isPinpad() const {return false;}
};

