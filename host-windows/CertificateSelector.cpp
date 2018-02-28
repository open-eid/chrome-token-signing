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
#include "PKCS11CertificateSelector.h"
#include "PKCS11Path.h"
#include "Labels.h"

#include <cryptuiapi.h>

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

CertificateSelector* CertificateSelector::createCertificateSelector()
{
	PKCS11Path::Params p11 = PKCS11Path::getPkcs11ModulePath();
	if (!p11.path.empty())
		return new PKCS11CertificateSelector(p11.path);
	else
		return new NativeCertificateSelector();
}

bool CertificateSelector::isValid(PCCERT_CONTEXT cert, bool forSigning) const
{
	if (CertVerifyTimeValidity(nullptr, cert->pCertInfo) > 0)
		return false;
	BYTE keyUsage = 0;
	CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, cert->pCertInfo, &keyUsage, 1);
	return forSigning == (keyUsage & CERT_NON_REPUDIATION_KEY_USAGE) > 0;
}

std::vector<unsigned char> CertificateSelector::showDialog() const
{
	PCCERT_CONTEXT cert = CertEnumCertificatesInStore(store, nullptr);
	if (!cert)
		throw NoCertificatesException();
	CertFreeCertificateContext(cert);
	std::wstring title = Labels::l10n.get("select certificate");
	std::wstring text = Labels::l10n.get("cert info");
	CRYPTUI_SELECTCERTIFICATE_STRUCT pcsc = { sizeof(pcsc) };
	pcsc.pFilterCallback = nullptr;
	pcsc.pvCallbackData = nullptr;
	pcsc.szTitle = title.c_str();
	pcsc.szDisplayString = text.c_str();
	pcsc.cDisplayStores = 1;
	pcsc.rghDisplayStores = const_cast<HCERTSTORE*>(&store);
	cert = CryptUIDlgSelectCertificate(&pcsc);
	if (!cert)
		throw UserCancelledException();
	std::vector<unsigned char> data(cert->pbCertEncoded, cert->pbCertEncoded + cert->cbCertEncoded);
	CertFreeCertificateContext(cert);
	return data;
}