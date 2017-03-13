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

#include "CertificateSelector.h"
#include "PKCS11CardManager.h"
#include "DialogManager.h"
#include <future>
#include <string>
#include <vector>
#include "Logger.h"

class PKCS11CertificateSelector : public CertificateSelector {
public:
	PKCS11CertificateSelector(const string &_driverPath) : CertificateSelector(){
		driverPath = _driverPath;
		initialize();
	}
	std::string getCert();

private:
	void initialize();
	void fetchAllSigningCertificates();
	void addCertificateToMemoryStore(std::vector<unsigned char> signCert);
	PKCS11CardManager* createCardManager();

	HCERTSTORE  hMemoryStore;
	std::string driverPath;
};