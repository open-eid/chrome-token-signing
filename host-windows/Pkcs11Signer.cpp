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

#include "Pkcs11Signer.h"
#include "PKCS11CardManager.h"
#include "Logger.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
#include "DialogManager.h"
#include <future>
#include <string>

using namespace std;

void Pkcs11Signer::initialize() {
	cardManager = getCardManager();
	pinTriesLeft = cardManager->getPIN2RetryCount();
}

unique_ptr<PKCS11CardManager> Pkcs11Signer::getCardManager() {
	try {
		unique_ptr<PKCS11CardManager> manager;
		for (auto &token : createCardManager()->getAvailableTokens()) {
			manager.reset(createCardManager()->getManagerForReader(token));
			if (manager->getSignCert() == BinaryUtils::hex2bin(*getCertInHex())) {
				break;
			}
			manager.reset();
		}

		if (!manager) {
			_log("No card manager found for this certificate");
			throw InvalidArgumentException("No card manager found for this certificate");
		}
		return manager;
	}
	catch (const std::runtime_error &a) {
		_log("Technical error: %s",a.what());
		throw TechnicalException("Error getting certificate manager: " + string(a.what()));
	}
}

PKCS11CardManager* Pkcs11Signer::createCardManager() {
	if (pkcs11ModulePath.empty()) {
		return PKCS11CardManager::instance();
	}
	return PKCS11CardManager::instance(pkcs11ModulePath);
}

string Pkcs11Signer::sign() {
	_log("Signing using PKCS#11 module");
	_log("Hash is %s and cert is %s", getHash()->c_str(), getCertInHex()->c_str());
	validateHashLength();
	return askPinAndSignHash();
}

string Pkcs11Signer::askPinAndSignHash() {
	try {
		validatePinNotBlocked();
		char* signingPin = askPin();
		vector<unsigned char> binaryHash = BinaryUtils::hex2bin(*getHash());
		vector<unsigned char> result = cardManager->sign(binaryHash, signingPin);
		string signature = BinaryUtils::bin2hex(result);
		_log("Sign result: %s", signature.c_str());
		return signature;
	}
	catch (AuthenticationError &e) {
		_log("Wrong pin");
		handleWrongPinEntry();
		return askPinAndSignHash();
	}
	catch (AuthenticationBadInput &e) {
		_log("Bad pin input");
		handleWrongPinEntry();
		return askPinAndSignHash();
	}
}

char* Pkcs11Signer::askPin() {
	char* signingPin = dialog.getPin();
	if (strlen(signingPin) < 4) {
		_log("Pin is too short");
		dialog.showWrongPinError(pinTriesLeft);
		return askPin();
	}
	return signingPin;
}

void Pkcs11Signer::validateHashLength() {
	int length = getHash()->length();
	if (length != BINARY_SHA1_LENGTH * 2 && length != BINARY_SHA224_LENGTH * 2 && length != BINARY_SHA256_LENGTH * 2 && length != BINARY_SHA384_LENGTH * 2 && length != BINARY_SHA512_LENGTH * 2) {
		_log("Hash length %i is invalid", getHash()->length());
		throw InvalidHashException();
	}
}

void Pkcs11Signer::validatePinNotBlocked() {
	if (pinTriesLeft <= 0) {
		_log("PIN2 retry count is zero");
		dialog.showPinBlocked();
		throw PinBlockedException();
	}
}

void Pkcs11Signer::handleWrongPinEntry() {
	pinTriesLeft--;
	validatePinNotBlocked();
	dialog.showWrongPinError(pinTriesLeft);
}

void Pkcs11Signer::setPkcs11ModulePath(string &path) {
	pkcs11ModulePath = path;
}