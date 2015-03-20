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

#include "Signer.h"
#include "Logger.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
#include <Windows.h>
#include <ncrypt.h>
#include <WinCrypt.h>
#include <cryptuiapi.h>
#include <comdef.h>

using namespace std;

string Signer::sign() {

	BCRYPT_PKCS1_PADDING_INFO padInfo;
	vector<unsigned char> digest = BinaryUtils::hex2bin(hash.c_str());

	ALG_ID alg = 0;
	
	switch (digest.size())
	{
	case BINARY_SHA1_LENGTH:
		padInfo.pszAlgId = NCRYPT_SHA1_ALGORITHM;
		alg = CALG_SHA1;
		break;
	case BINARY_SHA224_LENGTH:
		padInfo.pszAlgId = L"SHA224";
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
	
	SECURITY_STATUS err = 0;
	DWORD size = 256;
	vector<unsigned char> signature(size, 0);

	HCERTSTORE store = CertOpenSystemStore(0, L"MY");
	if (!store) {
		throw TechnicalException("Failed to open Cert Store");
	}
	
	vector<unsigned char> certInBinary = BinaryUtils::hex2bin(certInHex.c_str());
	
	PCCERT_CONTEXT certFromBinary = CertCreateCertificateContext(X509_ASN_ENCODING, &certInBinary[0], certInBinary.size());
	PCCERT_CONTEXT certInStore = CertFindCertificateInStore(store, X509_ASN_ENCODING, 0, CERT_FIND_EXISTING, certFromBinary, 0);
	CertFreeCertificateContext(certFromBinary);

	if (!certInStore)
	{
		CertCloseStore(store, 0);
		throw NoCertificatesException();
	}

	DWORD flags = CRYPT_ACQUIRE_PREFER_NCRYPT_KEY_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG;
	DWORD spec = 0;
	BOOL freeKeyHandle = false;
	HCRYPTPROV_OR_NCRYPT_KEY_HANDLE key = NULL;
	BOOL gotKey = true;
	gotKey = CryptAcquireCertificatePrivateKey(certInStore, flags, 0, &key, &spec, &freeKeyHandle);


	//TODO: Remove temporary logging stuff
	_log("Key acquired: %s", gotKey ? "true" : "false");
	_log("Free Key Handle: %s", freeKeyHandle ? "true" : "false");
	_log("Spec: %s", spec == AT_SIGNATURE ? "AT_SIGNATURE" : "OTHER");

	if (spec == CERT_NCRYPT_KEY_SPEC) {

		err = NCryptSignHash(key, &padInfo, PBYTE(&digest[0]), DWORD(digest.size()),
			&signature[0], DWORD(signature.size()), (DWORD*)&size, BCRYPT_PAD_PKCS1);
		signature.resize(size);

		//Otherwise the key handle is realeased on the last free action of the certificate context.
		if (freeKeyHandle) {
			//How to release depends on if it's an Ncrypt key
			if (spec == CERT_NCRYPT_KEY_SPEC) {
				NCryptFreeObject(key);
			}
			else {
				CryptReleaseContext(key, 0);
			}
		}
		CertFreeCertificateContext(certInStore);
		CertCloseStore(store, 0);

		//TODO: Remove temporary logging stuff
		_com_error error(err);
		_bstr_t b(error.ErrorMessage());
		const char* c = b;
		_log(c);

		switch (err)
		{
		case ERROR_SUCCESS:
			return BinaryUtils::bin2hex(signature);
		case SCARD_W_CANCELLED_BY_USER:
			throw UserCancelledException("Signing was cancelled");
		case SCARD_W_CHV_BLOCKED:
			throw PinBlockedException();
		case NTE_INVALID_HANDLE:
			throw TechnicalException("The supplied handle is invalid");
		default:
			throw TechnicalException("Signing failed");
		}
	
	}
	else if (spec == AT_SIGNATURE) {

		HCRYPTHASH hash = 0;
		if (!CryptCreateHash(key, alg, 0, 0, &hash))
			throw TechnicalException("CreateHash failed");

		if (!CryptSetHashParam(hash, HP_HASHVAL, digest.data(), 0))
		{
			CryptDestroyHash(hash);
			throw TechnicalException("SetHashParam failed");
		}

		DWORD size = 256;

		if (CryptSignHashW(hash, AT_SIGNATURE, 0, 0, LPBYTE(signature.data()), &size))
			signature.resize(size);
		else
			signature.clear();
		switch (GetLastError())
		{
		case ERROR_CANCELLED: 
			throw UserCancelledException("CSP Signing was cancelled");
		default: break;
		}

		CryptDestroyHash(hash);
		return BinaryUtils::bin2hex(signature);
	}

}

