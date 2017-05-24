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
#include "HostExceptions.h"
#include "CngCapiSigner.h"
#include "Pkcs11Signer.h"
#include "PKCS11Path.h"

using namespace std;

Signer * Signer::createSigner(const jsonxx::Object &jsonRequest){
	string cert = jsonRequest.get<string>("cert");
	std::string pkcs11 = PKCS11Path::getPkcs11ModulePath();
	if (!pkcs11.empty()) {
		return new Pkcs11Signer(pkcs11, cert);
	}
	return new CngCapiSigner(cert);
}

bool Signer::showInfo(const string &msg)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), msg.size(), nullptr, 0);
	if (size > 500)
		throw TechnicalException("Information message to long");
	wstring wmsg(size, 0);
	if (MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), msg.size(), &wmsg[0], wmsg.size()) != size)
		throw TechnicalException("Failed to convert string to wide chars");
	return MessageBox(nullptr, wmsg.c_str(), L"", MB_OKCANCEL | MB_ICONINFORMATION | MB_SYSTEMMODAL) == IDOK;
}
