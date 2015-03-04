/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "CertificateSelection.h"
#include "BinaryUtils.h"
#include <Windows.h>
#include <ncrypt.h>
#include <WinCrypt.h>
#include <cryptuiapi.h>

using namespace std;

extern "C" {

	typedef BOOL(WINAPI * PFNCCERTDISPLAYPROC)(
		__in  PCCERT_CONTEXT pCertContext,
		__in  HWND hWndSelCertDlg,
		__in  void *pvCallbackData
		);

	typedef struct _CRYPTUI_SELECTCERTIFICATE_STRUCT {
		DWORD               dwSize;
		HWND                hwndParent;
		DWORD               dwFlags;
		LPCWSTR             szTitle;
		DWORD               dwDontUseColumn;
		LPCWSTR             szDisplayString;
		PFNCFILTERPROC      pFilterCallback;
		PFNCCERTDISPLAYPROC pDisplayCallback;
		void *              pvCallbackData;
		DWORD               cDisplayStores;
		HCERTSTORE *        rghDisplayStores;
		DWORD               cStores;
		HCERTSTORE *        rghStores;
		DWORD               cPropSheetPages;
		LPCPROPSHEETPAGEW   rgPropSheetPages;
		HCERTSTORE          hSelectedCertStore;
	} CRYPTUI_SELECTCERTIFICATE_STRUCT, *PCRYPTUI_SELECTCERTIFICATE_STRUCT;

	typedef const CRYPTUI_SELECTCERTIFICATE_STRUCT
		*PCCRYPTUI_SELECTCERTIFICATE_STRUCT;

	PCCERT_CONTEXT WINAPI CryptUIDlgSelectCertificateW(
		__in  PCCRYPTUI_SELECTCERTIFICATE_STRUCT pcsc
		);

#define CryptUIDlgSelectCertificate CryptUIDlgSelectCertificateW




}  // extern "C"

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

jsonxx::Object CertificateSelection::getCert() {
	jsonxx::Object json;
	HCERTSTORE store = CertOpenSystemStore(0, L"MY");
	if (!store) {
		return json << "returnCode" << 5 << "message" << "Failed to open Cert Store";
	}

	
	CRYPTUI_SELECTCERTIFICATE_STRUCT pcsc = { sizeof(pcsc) };
	pcsc.pFilterCallback = filter_proc;
	pcsc.pvCallbackData = nullptr;
	pcsc.cDisplayStores = 1;
	pcsc.rghDisplayStores = &store;
	PCCERT_CONTEXT cert_context = CryptUIDlgSelectCertificate(&pcsc);
	
	if (!cert_context)
		return json << "returnCode" << 1 << "message" << "User cancelled";

	vector<unsigned char> data(cert_context->pbCertEncoded, cert_context->pbCertEncoded + cert_context->cbCertEncoded);
	CertFreeCertificateContext(cert_context);
	CertCloseStore(store, 0);
	return json << "cert" << BinaryUtils::bin2hex(data);
}