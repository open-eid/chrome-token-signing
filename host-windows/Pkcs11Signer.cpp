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

using namespace std;

Pkcs11Signer::Pkcs11Signer(const string &pkcs11ModulePath, const string &certInHex)
	: Signer(certInHex)
{
	// Init static card manager
	pkcs11ModulePath.empty() ? PKCS11CardManager::instance() : PKCS11CardManager::instance(pkcs11ModulePath);
}

unique_ptr<PKCS11CardManager> Pkcs11Signer::getCardManager() {
	try {
		for (auto &token : PKCS11CardManager::instance()->getAvailableTokens()) {
			try {
				unique_ptr<PKCS11CardManager> manager(PKCS11CardManager::instance()->getManagerForReader(token));
				if (manager->getSignCert() == BinaryUtils::hex2bin(getCertInHex()))
					return manager;
			}
			catch (const PKCS11TokenNotRecognized &ex) {
				_log("%s", ex.what());
			}
			catch (const PKCS11TokenNotPresent &ex) {
				_log("%s", ex.what());
			}
		}
	}
	catch (const runtime_error &a) {
		_log("Technical error: %s",a.what());
		throw TechnicalException("Error getting certificate manager: " + string(a.what()));
	}
	_log("No card manager found for this certificate");
	throw InvalidArgumentException("No card manager found for this certificate");
}

vector<unsigned char> Pkcs11Signer::sign(const vector<unsigned char> &digest) {
	_log("Signing using PKCS#11 module");
	unique_ptr<PKCS11CardManager> manager = getCardManager();
	pinTriesLeft = manager->getPIN2RetryCount();

	try {
		validatePinNotBlocked();
		char* signingPin = askPin();
		return manager->sign(digest, signingPin);
	}
	catch (const AuthenticationError &) {
		_log("Wrong pin");
		handleWrongPinEntry();
		return sign(digest);
	}
	catch (const AuthenticationBadInput &) {
		_log("Bad pin input");
		handleWrongPinEntry();
		return sign(digest);
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