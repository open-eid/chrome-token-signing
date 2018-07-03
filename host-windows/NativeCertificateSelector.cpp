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
#include "Exceptions.h"
#include "Logger.h"

std::vector<unsigned char> NativeCertificateSelector::getCert(bool forSigning) const {
	HCERTSTORE sys = CertOpenStore(CERT_STORE_PROV_SYSTEM,
		X509_ASN_ENCODING, 0, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if (!sys)
		throw TechnicalException("Failed to open Cert Store");

	PCCERT_CONTEXT cert = nullptr;
	while (cert = CertEnumCertificatesInStore(sys, cert)) {
		if (!isValid(cert, forSigning))
			continue;
		DWORD flags = CRYPT_ACQUIRE_CACHE_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG | CRYPT_ACQUIRE_SILENT_FLAG | CRYPT_ACQUIRE_PREFER_NCRYPT_KEY_FLAG;
		HCRYPTPROV_OR_NCRYPT_KEY_HANDLE key = 0;
		DWORD spec = 0;
		BOOL freeKey = FALSE;
		CryptAcquireCertificatePrivateKey(cert, flags, 0, &key, &spec, &freeKey);
		if (!key)
			continue;
		switch (spec)
		{
		case CERT_NCRYPT_KEY_SPEC:
		{
			NCRYPT_PROV_HANDLE prov = 0;
			DWORD type = 0, size = sizeof(prov);
			NCryptGetProperty(key, NCRYPT_PROVIDER_HANDLE_PROPERTY, PBYTE(&prov), size, &size, 0);
			if (prov)
			{
				size = sizeof(type);
				NCryptGetProperty(prov, NCRYPT_IMPL_TYPE_PROPERTY, PBYTE(&type), size, &size, 0);
				NCryptFreeObject(prov);
			}
			if (freeKey)
				NCryptFreeObject(key);
			if ((type & (NCRYPT_IMPL_HARDWARE_FLAG | NCRYPT_IMPL_REMOVABLE_FLAG)) == 0)
			{
				_log("Key does not contain hardware or removable flag.");
				continue;
			}
			break;
		}
		case AT_KEYEXCHANGE:
		case AT_SIGNATURE:
		{
			DWORD type = 0;
			DWORD size = sizeof(type);
			CryptGetProvParam(key, PP_IMPTYPE, PBYTE(&type), &size, 0);
			if ((type & (CRYPT_IMPL_HARDWARE | CRYPT_IMPL_REMOVABLE)) == 0)
			{
				_log("Key does not contain hardware or removable flag.");
				continue;
			}
			if (freeKey)
				CryptReleaseContext(key, 0);
			break;
		}
		}
		PCCERT_CONTEXT copy = nullptr;
		if (CertAddCertificateContextToStore(store, cert, CERT_STORE_ADD_USE_EXISTING, &copy))
			_log("Certificate added to the memory store.");
		else
			_log("Could not add the certificate to the memory store.");
	}
	CertCloseStore(sys, 0);
	return showDialog();
}
