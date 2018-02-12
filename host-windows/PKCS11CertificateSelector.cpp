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
#include "Exceptions.h"

#include <memory>

using namespace std;

PKCS11CertificateSelector::PKCS11CertificateSelector(const string &_driverPath)
	: CertificateSelector()
	, driverPath(_driverPath)
{}

vector<unsigned char> PKCS11CertificateSelector::getCert(bool forSigning) const {
	int certificatesCount = 0;
	try {
		for (const auto &token : PKCS11CardManager(driverPath).tokens()) {
			PCCERT_CONTEXT cert = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, token.cert.data(), token.cert.size());
			if (!cert)
				continue;
			_log("new certificate handle created.");

			if (!isValid(cert, forSigning)) {
				CertFreeCertificateContext(cert);
				continue;
			}
			if (CertAddCertificateContextToStore(store, cert, CERT_STORE_ADD_USE_EXISTING, nullptr)) {
				++certificatesCount;
				_log("Certificate added to the memory store.");
			}
			else
				_log("Could not add the certificate to the memory store.");
		}
	}
	catch (const std::runtime_error &a) {
		_log("Technical error: %s", a.what());
		throw TechnicalException("Error getting certificate manager: " + string(a.what()));
	}
	if (certificatesCount < 1)
		throw NoCertificatesException();
	return showDialog();
}
