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

#include "Exceptions.h"
#include <afx.h>
#include <wincrypt.h>
#include <vector>

class CertificateSelector {
public:
	static CertificateSelector* createCertificateSelector();

	virtual ~CertificateSelector() {
		if (store)
			CertCloseStore(store, 0);
	}
	virtual std::vector<unsigned char> getCert(bool forSigning) const throw(UserCancelledException, TechnicalException) = 0;

protected:
	CertificateSelector() = default;
	bool isValid(PCCERT_CONTEXT cert, bool forSigning) const;
	std::vector<unsigned char> showDialog() const;
	HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, 0, NULL);
};