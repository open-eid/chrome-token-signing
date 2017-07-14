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
#include "Labels.h"
#include "Logger.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
#include "PinDialog.h"

#include <WinCrypt.h>

using namespace std;

Pkcs11Signer::Pkcs11Signer(const string &pkcs11ModulePath, const string &certInHex)
	: Signer(certInHex)
	, pkcs11Path(pkcs11ModulePath)
{
	HMODULE hModule = ::GetModuleHandle(NULL);
	if (hModule == NULL) {
		_log("MFC initialization failed. Module handle is null");
		throw TechnicalException("MFC initialization failed. Module handle is null");
	}
	// initialize MFC
	if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0)) {
		_log("MFC initialization failed");
		throw TechnicalException("MFC initialization failed");
	}
}

vector<unsigned char> Pkcs11Signer::sign(const vector<unsigned char> &digest) {
	_log("Signing using PKCS#11 module");

	PKCS11CardManager::Token selected;
	try {
		for (const PKCS11CardManager::Token &token : PKCS11CardManager::instance(pkcs11Path)->tokens()) {
			if (token.cert == BinaryUtils::hex2bin(getCertInHex())) {
				selected = token;
				break;
			}
		}
	}
	catch (const runtime_error &a) {
		_log("Technical error: %s", a.what());
		throw TechnicalException("Error getting certificate manager: " + string(a.what()));
	}

	if (selected.cert.empty()) {
		_log("No card manager found for this certificate");
		throw InvalidArgumentException("No card manager found for this certificate");
	}

	pinTriesLeft = selected.retry;

	try {
		validatePinNotBlocked();
		return PKCS11CardManager::instance(pkcs11Path)->sign(selected, digest, askPin());
	}
	catch (const AuthenticationError &) {
		_log("Wrong pin");
		pinTriesLeft--;
		handleWrongPinEntry();
		return sign(digest);
	}
	catch (const AuthenticationBadInput &) {
		_log("Bad pin input");
		pinTriesLeft--;
		handleWrongPinEntry();
		return sign(digest);
	}
}

char* Pkcs11Signer::askPin() {
	_log("Showing pin entry dialog");
	wstring label = Labels::l10n.get("sign PIN");
	vector<unsigned char> data = BinaryUtils::hex2bin(getCertInHex());
	PCCERT_CONTEXT cert = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, data.data(), data.size());
	if (cert) {
		BYTE keyUsage = 0;
		CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, cert->pCertInfo, &keyUsage, 1);
		if (!(keyUsage & CERT_NON_REPUDIATION_KEY_USAGE))
			label = Labels::l10n.get("auth PIN");
		CertFreeCertificateContext(cert);
	}

	PinDialog dialog(label);
	if (dialog.DoModal() != IDOK) {
		_log("User cancelled");
		throw UserCancelledException();
	}
	if (strlen(dialog.getPin()) < 4) {
		_log("Pin is too short");
		handleWrongPinEntry();
		return askPin();
	}
	return dialog.getPin();
}

void Pkcs11Signer::validatePinNotBlocked() {
	if (pinTriesLeft <= 0) {
		_log("PIN2 retry count is zero");
		MessageBox(NULL, Labels::l10n.get("PIN2 blocked").c_str(), L"PIN Blocked", MB_OK | MB_ICONERROR);
		throw PinBlockedException();
	}
}

void Pkcs11Signer::handleWrongPinEntry() {
	validatePinNotBlocked();
	_log("Showing incorrect pin error dialog, %i tries left", pinTriesLeft);
	wstring msg = Labels::l10n.get("tries left") + L" " + to_wstring(pinTriesLeft);
	MessageBox(NULL, msg.c_str(), Labels::l10n.get("incorrect PIN2").c_str(), MB_OK | MB_ICONERROR);
}
