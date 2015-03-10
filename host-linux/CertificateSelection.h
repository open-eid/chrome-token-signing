/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#pragma once

#include "PKCS11CardManager.h"
#include "Logger.h"
#include "CertDialog.h"
#include "BinaryUtils.h"

using namespace jsonxx;

class CertificateSelection {
public:
    Object getCert() {
        try {
            std::unique_ptr<PKCS11CardManager> cardManager(new PKCS11CardManager);
            std::unique_ptr<CertDialog> dialog(new CertDialog());
            time_t currentTime = DateUtils::now();
            bool found = false;
            for (auto &token : cardManager->getAvailableTokens()) {
                CleverCardManager *manager = cardManager->getManagerForReader(token);
                time_t validTo = manager->getValidTo();
                if (currentTime <= validTo) {
                    dialog->addRow(token, manager->getCN(), manager->getType(), DateUtils::timeToString(validTo));
                    found = true;
                }
                delete manager;
            }

            if (!found)
                return Object() << "result" << "no_certificates";

            int result = dialog->run();
            dialog->hide();
            if (result != GTK_RESPONSE_OK)
                return Object() << "result" << "user_cancel";

            int readerId = dialog->getSelectedCertIndex();
            CleverCardManager *manager = cardManager->getManagerForReader(readerId);
            std::vector<unsigned char> cert = manager->getSignCert();

            _log("cert binary size = %i", cert.size());
            return Object() << "cert" << BinaryUtils::bin2hex(cert);
        } catch (const std::runtime_error &e) {
            _log(e.what());
        }
        return Object() << "result" << "technical_error";
    }
};
