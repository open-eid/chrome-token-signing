/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef MOCKCARDMANAGER_H
#define	MOCKCARDMANAGER_H

#include <gmock/gmock.h>
#include "CardManager.h"

class MockCardManager : public CardManager {
 private:
	int readerId;

 public:
	MOCK_METHOD0(getAvailableTokens, std::vector<unsigned int>());
	MOCK_METHOD0(isReaderPresent, bool());
	MOCK_METHOD0(getCardName, std::string());
	MOCK_METHOD0(getPersonalCode, std::string());
	MOCK_METHOD0(isCardInReader, bool());
	MOCK_METHOD0(getPIN2RetryCount, int());
	MOCK_METHOD2(sign, ByteVec(const ByteVec&, const PinString&));
	MOCK_METHOD0(getCN, std::string());
	MOCK_METHOD0(getType, std::string());
	MOCK_METHOD0(getValidTo, time_t());
	MOCK_METHOD0(getValidFrom, time_t());
	MOCK_METHOD0(getIssuerCN, std::string());
	MOCK_METHOD0(getCertSerialNumber, std::string());
	MOCK_METHOD0(getSignCert, ByteVec());
	MOCK_METHOD0(isPinpad, bool());

	bool parentIsReaderPresent() {
		return CardManager::isReaderPresent();
	}

	MockCardManager *getManagerForReader(int readerId) {
		this->readerId = readerId;
		return this;
	}
	
	int getReaderId() {
		return readerId;
	}
};

#endif	/* MOCKCARDMANAGER_H */

