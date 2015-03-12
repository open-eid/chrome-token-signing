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
#include "PinEntryDialog.h"
#include "PinPadDialog.h"
#include "BinaryUtils.h"
#include "Labels.h"
#include "Logger.h"
#include "error.h"

#include <string>

class Signer {
    PinDialog *pinDialog = nullptr;

    PinDialog *createPinDialog(PKCS11CardManager *manager) {
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

    jsonxx::Object error(int errorCode) {
        jsonxx::Object json;
        switch(errorCode)
        {
        case USER_CANCEL: json << "result" << "user_cancel"; break;
        case READER_NOT_FOUND: json << "result" << "no_certificates"; break;
        case CERT_NOT_FOUND: json << "result" << "no_certificates"; break;
        case INVALID_HASH: json << "result" << "invalid_argument"; break;
        case ONLY_HTTPS_ALLOWED: json << "result" << "not_allowed"; break;
        default: json << "result" << "technical_error";
        }
        return json;
    }

 public:

    jsonxx::Object sign(const std::string &hash, const std::string &cert) {
        std::unique_ptr<PKCS11CardManager> manager;
		int retriesLeft = 0;

		try {
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

            time_t currentTime = DateUtils::now();
            for (auto &token : PKCS11CardManager::instance()->getAvailableTokens()) {
                manager.reset(PKCS11CardManager::instance()->getManagerForReader(token));
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
                std::vector<unsigned char> signature = manager->sign(
                            BinaryUtils::hex2bin(hash),
                            PinString(pinDialog->getPin().c_str()));
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


	virtual ~Signer() {
        delete pinDialog;
	}
};
