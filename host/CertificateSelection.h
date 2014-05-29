/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef CERTIFICATEREQUEST_H
#define	CERTIFICATEREQUEST_H

#include "CardManager.h"
#include "Logger.h"
#include "CertDialog.h"
#include "Labels.h"
#include "ExtensionDialog.h"
#include "BinaryUtils.h"

class CertificateSelection : public ExtensionDialog {
 private:
	CertDialog *dialog;

 public:

	CertificateSelection() {
		cardManager = NULL;
		dialog = NULL;
#ifndef _TEST
		cardManager = new PKCS11CardManager();
		dialog = new CertDialog();
#endif
	}

	//unit tests
	CertificateSelection(CertDialog *certDialog, CleverCardManager *manager) {
		dialog = certDialog;
		cardManager = manager;
	}

	jsonxx::Object getCert() {
		try {
			time_t currentTime = now();
			std::vector<unsigned int> availableTokens = cardManager->getAvailableTokens();
			for (auto &token : availableTokens) {
				CleverCardManager *manager = cardManager->getManagerForReader(token);

				if (manager->isCardInReader()) {
					time_t validTo = manager->getValidTo();

					if (currentTime <= validTo)
						dialog->addRow(token, manager->getCN(), manager->getType(), DateUtils::timeToString(validTo));
				}
				FREE_MANAGER;
			}

			int result = dialog->run();
			dialog->hide();
			if (result != GTK_RESPONSE_OK) {
				return error(USER_CANCEL);
			}

			int readerId = dialog->getSelectedCertIndex();
			CleverCardManager *manager = cardManager->getManagerForReader(readerId);
			ByteVec cert = manager->getSignCert();

			std::string certHex = std::string(BinaryUtils::bin2hex(cert));
			std::string md5hex = std::string(BinaryUtils::bin2hex(BinaryUtils::md5(cert)));
			_log("cert binary size = %i md5 = %s", cert.size(), md5hex.c_str());
			jsonxx::Object json;
			json << "id" << md5hex <<
					"cert" << certHex <<
					"validFrom" << DateUtils::timeToString(manager->getValidFrom()) <<
					"validTo" << DateUtils::timeToString(manager->getValidTo()) <<
					"CN" << manager->getCN() <<
					"type" << manager->getType() <<
					"keyUsage" << manager->getKeyUsage() <<
					"issuerCN" << manager->getIssuerCN() <<
					"certSerialNumber" << manager->getCertSerialNumber() <<
					"certificateAsHex" << certHex;

			FREE_MANAGER;
			return json;
		} catch (std::runtime_error &e) {
			_log(e.what());
			return error(UNKNOWN_ERROR);
		}
	}
};
#endif	/* CERTIFICATEREQUEST_H */

