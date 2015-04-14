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

#pragma once

#include "SignerFactory.h"
#include "CngCapiSigner.h"
#include "Pkcs11Signer.h"
#include <string>

Signer * SignerFactory::createSigner(const jsonxx::Object &jsonRequest){
	string hashFromStdIn = jsonRequest.get<string>("hash");
	string cert = jsonRequest.get<string>("cert");
	bool usePkcs11Module = true;//TODO replace with extension parameter
	if (usePkcs11Module) {
		return new Pkcs11Signer(hashFromStdIn, cert);
	}
	else {
		return new CngCapiSigner(hashFromStdIn, cert);
	}
}
