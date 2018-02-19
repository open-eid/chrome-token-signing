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
#include "HostExceptions.h"
#include "PinDialog.h"

#include <WinCrypt.h>

#include <memory>

using namespace std;

Pkcs11Signer::Pkcs11Signer(const string &pkcs11ModulePath, const vector<unsigned char> &cert)
	: Signer(cert)
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
	std::unique_ptr<PKCS11CardManager> pkcs11;
	try {
		pkcs11.reset(new PKCS11CardManager(pkcs11Path));
		for (const PKCS11CardManager::Token &token : pkcs11->tokens()) {
			if (token.cert == cert) {
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
		return pkcs11->sign(selected, digest, askPin());
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
	if (PCCERT_CONTEXT certificate = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, cert.data(), cert.size())) {
		BYTE keyUsage = 0;
		CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certificate->pCertInfo, &keyUsage, 1);
		if (!(keyUsage & CERT_NON_REPUDIATION_KEY_USAGE))
			label = Labels::l10n.get("auth PIN");
		CertFreeCertificateContext(certificate);
	}
	size_t start_pos = 0;
	while((start_pos = label.find(L"@PIN@", start_pos)) != std::string::npos) {
		label.replace(start_pos, 5, L"PIN");
		start_pos += 3;
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
