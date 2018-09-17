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
#include "PinDialog.h"
#include "PKCS11CardManager.h"
#include "Labels.h"
#include "Logger.h"
#include "Exceptions.h"

#include <WinCrypt.h>

using namespace std;

Pkcs11Signer::Pkcs11Signer(const string &pkcs11ModulePath, const vector<unsigned char> &cert)
	: Signer(cert)
	, pkcs11Path(pkcs11ModulePath)
{
}

vector<unsigned char> Pkcs11Signer::sign(const vector<unsigned char> &digest)
{
	_log("Signing using PKCS#11 module");

	PKCS11CardManager::Token selected;
	PKCS11CardManager pkcs11(pkcs11Path);
	for (const PKCS11CardManager::Token &token : pkcs11.tokens()) {
		if (token.cert == cert) {
			selected = token;
			break;
		}
	}
	if (selected.cert.empty()) {
		_log("No card manager found for this certificate");
		throw InvalidArgumentException("No card manager found for this certificate");
	}
	if (selected.retry <= 0) {
		_log("PIN retry count is zero");
		MessageBox(nullptr, Labels::l10n.get("PIN2 blocked").c_str(), L"PIN Blocked", MB_OK | MB_ICONERROR);
		throw PinBlockedException();
	}

	wstring label = Labels::l10n.get("sign PIN");
	if (PCCERT_CONTEXT certificate = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, cert.data(), cert.size())) {
		BYTE keyUsage = 0;
		CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certificate->pCertInfo, &keyUsage, 1);
		if (!(keyUsage & CERT_NON_REPUDIATION_KEY_USAGE))
			label = Labels::l10n.get("auth PIN");
		CertFreeCertificateContext(certificate);
	}
	size_t start_pos = 0;
	while ((start_pos = label.find(L"@PIN@", start_pos)) != std::string::npos) {
		label.replace(start_pos, 5, L"PIN");
		start_pos += 3;
	}

	bool isInitialCheck = true;
	for (int pinTriesLeft = selected.retry; pinTriesLeft > 0;)
	{
		try {
			wstring msg;
			if (pinTriesLeft < 3)
			{
				if (!isInitialCheck)
					msg = Labels::l10n.get("incorrect PIN2");
				msg += Labels::l10n.get("tries left") + L" " + to_wstring(pinTriesLeft);
			}
			_log("Showing pin entry dialog");
			std::string pin = PinDialog::getPin(label, msg);
			if (pin.empty()) {
				_log("User cancelled");
				throw UserCancelledException();
			}
			return pkcs11.sign(selected, digest, pin.c_str());
		}
		catch (const AuthenticationError &) {
			_log("Wrong pin");
			pinTriesLeft--;
		}
		catch (const PinBlockedException &) {
			_log("PIN blocked");
			pinTriesLeft = 0;
		}
		catch (const AuthenticationBadInput &) {
			_log("Bad pin input");
		}
		isInitialCheck = false;
	}
	_log("PIN retry count is zero");
	MessageBox(nullptr, Labels::l10n.get("PIN2 blocked").c_str(), L"PIN Blocked", MB_OK | MB_ICONERROR);
	throw PinBlockedException();
}
