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

#include "CertificateSelectorFactory.h"
#include "NativeCertificateSelector.h"
#include "PKCS11CertificateSelector.h"
#include "AtrFetcher.h"
#include "PKCS11ModulePath.h"
#include "Logger.h"

#include <string>

CertificateSelector * CertificateSelectorFactory::createCertificateSelector(){

	AtrFetcher * atrFetcher = new AtrFetcher();
	std::vector<std::string> atrs = atrFetcher->fetchAtr();

	for (int i = 0; i < atrs.size(); i++) {
		if (PKCS11ModulePath::isKnownAtr(atrs[i])) {
			return new PKCS11CertificateSelector(PKCS11ModulePath::getModulePath());
		}
	}
	return new NativeCertificateSelector();
}
