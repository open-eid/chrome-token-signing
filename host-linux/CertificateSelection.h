/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
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
		cardManager = new PKCS11CardManager(PKCS11_MODULE);
		dialog = new CertDialog();
#endif
	}

	//unit tests
	CertificateSelection(CertDialog *certDialog, CleverCardManager *manager) {
		dialog = certDialog;
		cardManager = manager;
	}

	jsonxx::Object getCert() {
                jsonxx::Object json;
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
				return jsonxx::Object() << "result" << "user_cancel";
			}

			int readerId = dialog->getSelectedCertIndex();
			CleverCardManager *manager = cardManager->getManagerForReader(readerId);
			std::vector<unsigned char> cert = manager->getSignCert();

			_log("cert binary size = %i", cert.size());
			FREE_MANAGER;
			return jsonxx::Object() << "cert" << BinaryUtils::bin2hex(cert);;
		} catch (std::runtime_error &e) {
			_log(e.what());
		}
		return jsonxx::Object() << "result" << "technical_error";
	}
};
#endif	/* CERTIFICATEREQUEST_H */

