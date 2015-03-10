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

#ifdef _TEST
#include "../test/MockCardManager.h"
typedef MockCardManager CleverCardManager;
#define FREE_MANAGER
#else
#include "PKCS11CardManager.h"
typedef PKCS11CardManager CleverCardManager;
#endif

#include <string>
#include "PinEntryDialog.h"
#include "PinPadDialog.h"
#include "BinaryUtils.h"
#include "Labels.h"
#include "Logger.h"
#include "ExtensionDialog.h"
#include "error.h"

class Signer : public ExtensionDialog {
 private:
	std::string hash;
	std::string cert;
    PinDialog *pinDialog = nullptr;

	PinDialog *createPinDialog(CleverCardManager *manager) {
		if (pinDialog != NULL) {
			return pinDialog;
		}

		if (manager->isPinpad()) {
			_log("PINPAD: YES");
			return new PinPadDialog();
		}
		else {
			_log("PINPAD: NO");
			return new PinEntryDialog();
		}
	}

 public:

	Signer(std::string hash, std::string cert) : hash(hash), cert(cert) {
#ifndef _TEST
        cardManager = new PKCS11CardManager;
#endif
	}

	//Used for testing
	Signer(std::string hash, std::string cert, PinDialog *dialog, CleverCardManager *manager) : hash(hash), cert(cert) {
		cardManager = manager;
		pinDialog = dialog;
	}

	jsonxx::Object sign() {
        std::unique_ptr<CleverCardManager> manager;
		int retriesLeft = 0;

		try {
			checkHash();

            time_t currentTime = DateUtils::now();
            for (auto &token : cardManager->getAvailableTokens()) {
                manager.reset(cardManager->getManagerForReader(token));
                if (manager->getSignCert() == BinaryUtils::hex2bin(cert) &&
                    currentTime <= manager->getValidTo()) {
                    _log("Got readerId %i from certId ", token);
                    break;
                }
                manager.reset();
            }

            if(!manager)
                return jsonxx::Object() << "result" << "invalid_argument";

            retriesLeft = manager->getPIN2RetryCount();
            if (retriesLeft == 0)
                return jsonxx::Object() << "result" << "pin_blocked";

            pinDialog = createPinDialog(manager.get());
            retriesLeftMessage(retriesLeft, true);
			pinDialog->setCardInfo(manager->getCardName() + ", " + manager->getPersonalCode());
        } catch (const CommonError &e) {
			_log("%s", e.what());
			return error(e.code);
        } catch (const std::runtime_error &e) {
			_log("%s", e.what());
            return jsonxx::Object() << "result" << "technical_error";
        }

		do {
			try {
				PinString pin(pinDialog->getPin().c_str());
                std::vector<unsigned char> hash = BinaryUtils::hex2bin(this->hash);
				std::vector<unsigned char> signature = manager->sign(hash, pin);
				pinDialog->hide();
                return jsonxx::Object() << "signature" << BinaryUtils::bin2hex(signature);
            } catch (const AuthenticationError &ae) {
				if (ae.aborted) {
					pinDialog->hide();
                    return error(USER_CANCEL);
				}
				if (!ae.badInput) {
					retriesLeft--;
				}
				retriesLeftMessage(retriesLeft);
				if (retriesLeft == 0) {
					pinDialog->hide();
				}
            } catch (const UserCanceledError &e) {
				pinDialog->hide();
                return error(USER_CANCEL);
            } catch (const CommonError &e) {
				_log("%s", e.what());
				pinDialog->hide();
				return error(e.code);
            } catch (const std::runtime_error &e) {
				pinDialog->hide();
				_log("%s", e.what());
                return error(UNKNOWN_ERROR);
			}
        } while (retriesLeft > 0);
        return jsonxx::Object() << "result" << "pin_blocked";
    }

	virtual void retriesLeftMessage(int retriesLeft, bool isInitialCheck = false) {
		_log("Retry count: %i", retriesLeft);
		if (retriesLeft < 3) {
			std::string errorMessage = l10nLabels.get("tries left") + std::to_string(retriesLeft);
			if (!isInitialCheck) {
				errorMessage = l10nLabels.get("incorrect PIN2") + errorMessage;
			}
			pinDialog->setErrorMessage(errorMessage);
			return;
		}
	}

	virtual void checkHash() {
        switch(hash.length())
        {
        case BINARY_SHA1_LENGTH * 2:
        case BINARY_SHA224_LENGTH * 2:
        case BINARY_SHA256_LENGTH * 2:
        case BINARY_SHA384_LENGTH * 2:
        case BINARY_SHA512_LENGTH * 2: break;
        default:
			throw InvalidHashError();
		}
	}

	virtual ~Signer() {
        delete pinDialog;
        delete cardManager;
	}
};
