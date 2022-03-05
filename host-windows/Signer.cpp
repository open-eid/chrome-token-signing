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
#include "Exceptions.h"
#include "NativeSigner.h"
#include "Pkcs11Signer.h"
#include "PKCS11Path.h"

#include <Windows.h>

using namespace std;

std::unique_ptr<Signer> Signer::createSigner(const vector<unsigned char> &cert) {
	if (PCCERT_CONTEXT certificate = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, cert.data(), cert.size())) {
		BYTE keyUsage = 0;
		CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certificate->pCertInfo, &keyUsage, 1);
		CertFreeCertificateContext(certificate);
		if ((keyUsage & CERT_NON_REPUDIATION_KEY_USAGE) == 0)
			throw InvalidArgumentException();
	}
	else
		throw InvalidArgumentException();

	PKCS11Path::Params p11 = PKCS11Path::getPkcs11ModulePath();
	if (!p11.path.empty()) {
		return std::unique_ptr<Signer>(new Pkcs11Signer(p11.path, cert));
	}
	return std::unique_ptr<Signer>(new NativeSigner(cert));
}

bool Signer::showInfo(const string &msg)
{
	if (msg.empty())
		return true;
	int size = MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), DWORD(msg.size()), nullptr, 0);
	if (size > 500)
		throw TechnicalException("Information message to long");
	wstring wmsg(size, 0);
	if (MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), DWORD(msg.size()), &wmsg[0], DWORD(wmsg.size())) != size)
		throw TechnicalException("Failed to convert string to wide chars");
	return MessageBox(nullptr, wmsg.c_str(), L"", MB_OKCANCEL | MB_ICONINFORMATION | MB_SYSTEMMODAL) == IDOK;
}
