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

#include "NativeSigner.h"
#include "HostExceptions.h"
#include "Logger.h"

#include <Windows.h>
#include <ncrypt.h>
#include <WinCrypt.h>
#include <cryptuiapi.h>

#include <memory>

using namespace std;

vector<unsigned char> NativeSigner::sign(const vector<unsigned char> &digest)
{
	BCRYPT_PKCS1_PADDING_INFO padInfo;
	DWORD obtainKeyStrategy = CRYPT_ACQUIRE_PREFER_NCRYPT_KEY_FLAG;

	ALG_ID alg = 0;	
	switch (digest.size())
	{
	case BINARY_SHA1_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA1_ALGORITHM;
		alg = CALG_SHA1;
		break;
	case BINARY_SHA224_LENGTH:
		padInfo.pszAlgId = L"SHA224";
		obtainKeyStrategy = CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG;
		break;
	case BINARY_SHA256_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA256_ALGORITHM;
		alg = CALG_SHA_256;
		break;
	case BINARY_SHA384_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA384_ALGORITHM;
		alg = CALG_SHA_384;
		break;
	case BINARY_SHA512_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA512_ALGORITHM;
		alg = CALG_SHA_512;
		break;
	default:
		throw InvalidHashException();
	}
	
	HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM,
		X509_ASN_ENCODING, 0, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if (!store)
		throw TechnicalException("Failed to open Cert Store");
	
	PCCERT_CONTEXT certFromBinary = CertCreateCertificateContext(X509_ASN_ENCODING, cert.data(), cert.size());
	PCCERT_CONTEXT certInStore = CertFindCertificateInStore(store, X509_ASN_ENCODING, 0, CERT_FIND_EXISTING, certFromBinary, 0);
	CertFreeCertificateContext(certFromBinary);
	CertCloseStore(store, 0);

	if (!certInStore)
		throw NoCertificatesException();

	DWORD flags = obtainKeyStrategy | CRYPT_ACQUIRE_COMPARE_KEY_FLAG;
	DWORD spec = 0;
	BOOL freeKeyHandle = false;
	HCRYPTPROV_OR_NCRYPT_KEY_HANDLE *handle = new HCRYPTPROV_OR_NCRYPT_KEY_HANDLE(0);
	BOOL gotKey = CryptAcquireCertificatePrivateKey(certInStore, flags, 0, handle, &spec, &freeKeyHandle);
	CertFreeCertificateContext(certInStore);
	if (!gotKey)
		throw NoCertificatesException();

	auto deleter = [=](HCRYPTPROV_OR_NCRYPT_KEY_HANDLE *key) {
		if (!freeKeyHandle)
			return;
		if (spec == CERT_NCRYPT_KEY_SPEC)
			NCryptFreeObject(*key);
		else
			CryptReleaseContext(*key, 0);
	};
	unique_ptr<HCRYPTPROV_OR_NCRYPT_KEY_HANDLE, decltype(deleter)> key(handle, deleter);

	SECURITY_STATUS err = ERROR_SUCCESS;
	vector<unsigned char> signature;
	switch (spec)
	{
	case CERT_NCRYPT_KEY_SPEC:
	{
		DWORD size = 0;
		err = NCryptSignHash(*key.get(), &padInfo, PBYTE(digest.data()), DWORD(digest.size()),
			nullptr, 0, LPDWORD(&size), BCRYPT_PAD_PKCS1);
		if (FAILED(err))
			break;

		signature.resize(size);
		err = NCryptSignHash(*key.get(), &padInfo, PBYTE(digest.data()), DWORD(digest.size()),
			signature.data(), DWORD(signature.size()), LPDWORD(&size), BCRYPT_PAD_PKCS1);
		break;
	}
	case AT_KEYEXCHANGE:
	case AT_SIGNATURE:
	{
		HCRYPTHASH hash = 0;
		if (!CryptCreateHash(*key.get(), alg, 0, 0, &hash))
			throw TechnicalException("CreateHash failed");
		if (!CryptSetHashParam(hash, HP_HASHVAL, digest.data(), 0))	{
			CryptDestroyHash(hash);
			throw TechnicalException("SetHashParam failed");
		}

		DWORD size = 0;
		if (!CryptSignHashW(hash, spec, nullptr, 0, nullptr, &size)) {
			CryptDestroyHash(hash);
			err = GetLastError();
			break;
		}

		signature.resize(size);
		if (!CryptSignHashW(hash, spec, nullptr, 0, LPBYTE(signature.data()), &size))
			err = GetLastError();
		CryptDestroyHash(hash);
		reverse(signature.begin(), signature.end());
		break;
	}
	default:
		throw TechnicalException("Incompatible key");
	}

	_log("sign return code: %x", err);
	switch (err)
	{
	case ERROR_SUCCESS:
		return signature;
	case SCARD_W_CANCELLED_BY_USER:
	case ERROR_CANCELLED:
		throw UserCancelledException("Signing was cancelled");
	case SCARD_W_CHV_BLOCKED:
		throw PinBlockedException();
	case NTE_INVALID_HANDLE:
		throw TechnicalException("The supplied handle is invalid");
	default:
		throw TechnicalException("Signing failed");
	}
}

