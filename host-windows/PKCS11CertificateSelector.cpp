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

#include "PKCS11CertificateSelector.h"
#include "PKCS11CardManager.h"
#include "Logger.h"
#include "HostExceptions.h"

#include <memory>

using namespace std;

PKCS11CertificateSelector::PKCS11CertificateSelector(const string &_driverPath)
	: CertificateSelector()
	, driverPath(_driverPath)
{}

vector<unsigned char> PKCS11CertificateSelector::getCert(bool forSigning) const {	
	HCERTSTORE  hMemoryStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, 0, NULL);
	if (hMemoryStore) {
		_log("Opened a memory store.");
	}
	else {
		_log("Error opening a memory store.");
		throw TechnicalException("Error opening a memory store.");
	}

	try {
		for (const auto &token : PKCS11CardManager::instance(driverPath)->tokens()) {
			PCCERT_CONTEXT cert = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, token.cert.data(), token.cert.size());
			if (!cert)
				continue;
			_log("new certificate handle created.");

			BYTE keyUsage = 0;
			CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, cert->pCertInfo, &keyUsage, 1);
			if (CertVerifyTimeValidity(NULL, cert->pCertInfo) != 0 ||
				(forSigning && !(keyUsage & CERT_NON_REPUDIATION_KEY_USAGE)) ||
				(!forSigning && keyUsage & CERT_NON_REPUDIATION_KEY_USAGE)) {
				CertFreeCertificateContext(cert);
				continue;
			}
			if (CertAddCertificateContextToStore(hMemoryStore, cert, CERT_STORE_ADD_USE_EXISTING, NULL)) {
				_log("Certificate added to the memory store.");
			}
			else {
				_log("Could not add the certificate to the memory store.");
			}
		}
	}
	catch (const std::runtime_error &a) {
		_log("Technical error: %s", a.what());
		CertCloseStore(hMemoryStore, 0);
		throw TechnicalException("Error getting certificate manager: " + string(a.what()));
	}

	return showDialog(hMemoryStore, nullptr);
}
