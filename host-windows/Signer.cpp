/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "Signer.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"
#include <Windows.h>
#include <ncrypt.h>
#include <WinCrypt.h>
#include <cryptuiapi.h>

using namespace std;

string Signer::sign() {
	checkHash();
	BCRYPT_PKCS1_PADDING_INFO padInfo;
	vector<unsigned char> digest = BinaryUtils::hex2bin(hash.c_str());
	
	switch (digest.size())
	{
	case 20:
		padInfo.pszAlgId = NCRYPT_SHA1_ALGORITHM;
		break;
	case 28:
		padInfo.pszAlgId = L"SHA224";
		break;
	case 32:
		padInfo.pszAlgId = NCRYPT_SHA256_ALGORITHM;
		break;
	case 48:
		padInfo.pszAlgId = NCRYPT_SHA384_ALGORITHM;
		break;
	case 64:
		padInfo.pszAlgId = NCRYPT_SHA512_ALGORITHM;
		break;
	default:
		padInfo.pszAlgId = nullptr;
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
		throw NoCertificatesException();
	}

	DWORD flags = CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG;
	DWORD spec = 0;
	BOOL ncrypt = false;
	NCRYPT_KEY_HANDLE key= NULL;
	CryptAcquireCertificatePrivateKey(certInStore, flags, 0, &key, &spec, &ncrypt);
	CertFreeCertificateContext(certInStore);

	err = NCryptSignHash(key, &padInfo, PBYTE(&digest[0]), DWORD(digest.size()),
		&signature[0], DWORD(signature.size()), (DWORD*)&size, BCRYPT_PAD_PKCS1);
	signature.resize(size);

	switch (err)
	{
		//TODO handle other smart card errors
	case ERROR_SUCCESS: 
		return BinaryUtils::bin2hex(signature);
	case SCARD_W_CANCELLED_BY_USER:
		throw UserCancelledException("Signing was cancelled");
	default:
		throw TechnicalException("Signing failed");
	}
}

void Signer::checkHash() {
	switch (hash.length())
	{
	case BINARY_SHA1_LENGTH * 2:
	case BINARY_SHA224_LENGTH * 2:
	case BINARY_SHA256_LENGTH * 2:
	case BINARY_SHA384_LENGTH * 2:
	case BINARY_SHA512_LENGTH * 2: break;
	default:
		throw InvalidHashException("Invalid Hash");
	}
}