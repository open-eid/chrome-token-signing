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
#include <vector>

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

std::vector<unsigned char> CertificateSelector::showDialog(HCERTSTORE store, PFNCFILTERPROC filter_proc) const
{
	std::wstring title = Labels::l10n.get("select certificate");
	std::wstring text = Labels::l10n.get("cert info");
	CRYPTUI_SELECTCERTIFICATE_STRUCT pcsc = { sizeof(pcsc) };
	pcsc.pFilterCallback = filter_proc;
	pcsc.pvCallbackData = nullptr;
	pcsc.szTitle = title.c_str();
	pcsc.szDisplayString = text.c_str();
	pcsc.cDisplayStores = 1;
	pcsc.rghDisplayStores = &store;
	PCCERT_CONTEXT cert = CryptUIDlgSelectCertificate(&pcsc);
	CertCloseStore(store, 0);
	if (!cert)
		throw UserCancelledException();
	std::vector<unsigned char> data(cert->pbCertEncoded, cert->pbCertEncoded + cert->cbCertEncoded);
	CertFreeCertificateContext(cert);
	return data;
}