#include "NativeCertificateSelector.h"
#include "BinaryUtils.h"
#include "HostExceptions.h"

using namespace std;


BOOL isCardInReader(PCCERT_CONTEXT certContext) {
	DWORD flags = CRYPT_ACQUIRE_CACHE_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG | CRYPT_ACQUIRE_SILENT_FLAG;
	NCRYPT_KEY_HANDLE key = 0;
	DWORD spec = 0;
	BOOL ncrypt = FALSE;
	CryptAcquireCertificatePrivateKey(certContext, flags, 0, &key, &spec, &ncrypt);
	if (!key) {
		return FALSE;
	}
	if (ncrypt) {
		NCryptFreeObject(key);
	}
	return TRUE;
}

BOOL WINAPI isValidForSigning(PCCERT_CONTEXT certContext) {
	BYTE keyUsage;
	CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certContext->pCertInfo, &keyUsage, 1);
	if (!(keyUsage & CERT_NON_REPUDIATION_KEY_USAGE)) {
		return FALSE;
	}
	if (CertVerifyTimeValidity(NULL, certContext->pCertInfo) != 0) {
		return FALSE;
	}

	return isCardInReader(certContext);
}

BOOL WINAPI filter_proc(PCCERT_CONTEXT certContext, BOOL *pfInitialSelectedCert, void *pvCallbackData) {
	return isValidForSigning(certContext);
}

string NativeCertificateSelector::getCert() {
	HCERTSTORE store = CertOpenSystemStore(0, L"MY");
	if (!store)
	{
		throw TechnicalException("Failed to open Cert Store");
	}

	PCCERT_CONTEXT pCertContextForEnumeration = nullptr;
	int certificatesCount = 0;
	while (pCertContextForEnumeration = CertEnumCertificatesInStore(store, pCertContextForEnumeration)) {
		if (isValidForSigning(pCertContextForEnumeration)) {
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

	CRYPTUI_SELECTCERTIFICATE_STRUCT pcsc = { sizeof(pcsc) };
	pcsc.pFilterCallback = filter_proc;
	pcsc.pvCallbackData = nullptr;
	pcsc.cDisplayStores = 1;
	pcsc.rghDisplayStores = &store;
	PCCERT_CONTEXT cert_context = CryptUIDlgSelectCertificate(&pcsc);

	if (!cert_context)
	{
		CertCloseStore(store, 0);
		throw UserCancelledException();
	}

	vector<unsigned char> data(cert_context->pbCertEncoded, cert_context->pbCertEncoded + cert_context->cbCertEncoded);
	CertFreeCertificateContext(cert_context);
	CertCloseStore(store, 0);
	return BinaryUtils::bin2hex(data);
}