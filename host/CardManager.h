/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef CARDMANAGER_H
#define	CARDMANAGER_H

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA512_LENGTH 64

#include "PinString.h"

class CardManager {
 public:
	virtual std::vector<unsigned int> getAvailableTokens() {return std::vector<unsigned int>();}
	virtual bool isReaderPresent() {
		return getAvailableTokens().size() > 0;
	}
	virtual bool isCardInReader() {return false;}
	CardManager *getManagerForReader(int readerId) {return new CardManager();}
	virtual std::vector<unsigned char> sign(const std::vector<unsigned char> &hash, const PinString &pin){return std::vector<unsigned char>();}
  virtual	std::string getCardName(){return "Mari-Liis Mannik";}
 	virtual	std::string getPersonalCode(){return "47101010033";}
  virtual int getPIN2RetryCount(){return 3;}
  virtual std::vector<unsigned char> getSignCert() {return std::vector<unsigned char>();}
	virtual std::string getCN() {return "";}
	virtual std::string getType() {return "";}
	virtual time_t getValidTo() {return 0;}
	virtual time_t getValidFrom() {return 0;}
	std::string getKeyUsage() {return "Non-Repudiation";}
	virtual std::string getIssuerCN() {return "";}
	virtual std::string getCertSerialNumber() {return "";}
	virtual bool isPinpad() {return false;}
};

#endif	/* CARDMANAGER_H */

