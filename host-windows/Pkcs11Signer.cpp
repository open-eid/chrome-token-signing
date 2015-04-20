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

void Pkcs11Signer::validateHashLength() {
	int length = getHash()->length();
	if (length != BINARY_SHA1_LENGTH * 2 && length != BINARY_SHA224_LENGTH * 2 && length != BINARY_SHA256_LENGTH * 2 && length != BINARY_SHA384_LENGTH * 2 && length != BINARY_SHA512_LENGTH * 2) {
		_log("Hash length %i is invalid", getHash()->length());
		throw InvalidHashException();
	}
}

unique_ptr<PKCS11CardManager> Pkcs11Signer::getCardManager() {
	unique_ptr<PKCS11CardManager> manager;

	try {
		for (auto &token : PKCS11CardManager::instance()->getAvailableTokens()) {
			manager.reset(PKCS11CardManager::instance()->getManagerForReader(token));
			if (manager->getSignCert() == BinaryUtils::hex2bin(*getCertInHex())) {
				break;
			}
			manager.reset();
		}

		if (!manager) {
			_log("No card manager found for this certificate");
			throw InvalidArgumentException("No card manager found for this certificate");
		}
	}
	catch (const std::runtime_error &a) {
		_log("Technical error: %s",a.what());
		throw TechnicalException("Error getting certificate manager: " + string(a.what()));
	}
	return manager;
}

string Pkcs11Signer::sign() {
	_log("Signing using PKCS#11 module");
	_log("Hash is %s and cert is %s", getHash()->c_str(), getCertInHex()->c_str());
	validateHashLength();
	unique_ptr<PKCS11CardManager> manager = getCardManager();
	vector<unsigned char> result;
	DialogManager dialog;
	char* signingPin = dialog.getPin();
	result = manager->sign(BinaryUtils::hex2bin(*getHash()), signingPin);
	string signature = BinaryUtils::bin2hex(result);
	_log("Sign result: %s", signature.c_str());
	return signature;
}