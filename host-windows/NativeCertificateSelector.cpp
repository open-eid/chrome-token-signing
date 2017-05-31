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

#include "NativeCertificateSelector.h"
#include "HostExceptions.h"

using namespace std;

static bool isValid(PCCERT_CONTEXT certContext, bool forSigning) {
	DWORD flags = CRYPT_ACQUIRE_CACHE_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG | CRYPT_ACQUIRE_SILENT_FLAG;
	NCRYPT_KEY_HANDLE key = 0;
	DWORD spec = 0;
	BOOL freeKey = FALSE;
	CryptAcquireCertificatePrivateKey(certContext, flags, 0, &key, &spec, &freeKey);
	if (!key) {
		return FALSE;
	}
	switch (spec)
	{
	case CERT_NCRYPT_KEY_SPEC:
		if (freeKey)
			NCryptFreeObject(key);
		break;
	case AT_KEYEXCHANGE:
	case AT_SIGNATURE:
	default:
		if (freeKey)
			CryptReleaseContext(key, 0);
		break;
	}
	BYTE keyUsage = 0;
	CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certContext->pCertInfo, &keyUsage, 1);
	return CertVerifyTimeValidity(NULL, certContext->pCertInfo) == 0 && (
		(forSigning && keyUsage & CERT_NON_REPUDIATION_KEY_USAGE) ||
		(!forSigning && keyUsage & CERT_DIGITAL_SIGNATURE_KEY_USAGE));
}

vector<unsigned char> NativeCertificateSelector::getCert(bool forSigning) const {
	HCERTSTORE store = CertOpenSystemStore(0, L"MY");
	if (!store) {
		throw TechnicalException("Failed to open Cert Store");
	}

	PCCERT_CONTEXT pCertContextForEnumeration = nullptr;
	int certificatesCount = 0;
	while (pCertContextForEnumeration = CertEnumCertificatesInStore(store, pCertContextForEnumeration)) {
		if (isValid(pCertContextForEnumeration, forSigning)) {
			certificatesCount++;
		}
	}
	if (pCertContextForEnumeration){
		CertFreeCertificateContext(pCertContextForEnumeration);
	}
	if (certificatesCount < 1) {
		CertCloseStore(store, 0);
		throw NoCertificatesException();
	}

	if (forSigning) {
		return showDialog(store, [](PCCERT_CONTEXT certContext, BOOL *pfInitialSelectedCert, void *pvCallbackData) -> BOOL {
			return isValid(certContext, true);
		});
	}
	else {
		return showDialog(store, [](PCCERT_CONTEXT certContext, BOOL *pfInitialSelectedCert, void *pvCallbackData) -> BOOL {
			return isValid(certContext, false);
		});
	}
}