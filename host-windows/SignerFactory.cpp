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

#include "SignerFactory.h"
#include "CngCapiSigner.h"
#include "Pkcs11Signer.h"
#include <string>

Signer * SignerFactory::createSigner(const jsonxx::Object &jsonRequest){
	string hashFromStdIn = jsonRequest.get<string>("hash");
	string cert = jsonRequest.get<string>("cert");
	bool usePkcs11Module = jsonRequest.has<bool>("forcePkcs11") && jsonRequest.get<bool>("forcePkcs11");
	if (usePkcs11Module) {
		Pkcs11Signer *signer = new Pkcs11Signer(hashFromStdIn, cert);
		if (jsonRequest.has<string>("pkcs11ModulePath")) {
			string pkcs11ModulePath = jsonRequest.get<string>("pkcs11ModulePath");
			signer->setPkcs11ModulePath(pkcs11ModulePath);
		}
		signer->initialize();
		return signer;
	}
	else {
		return new CngCapiSigner(hashFromStdIn, cert);
	}
}
