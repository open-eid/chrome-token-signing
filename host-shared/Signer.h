/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef SIGNER_H
#define	SIGNER_H

#ifdef _TEST
#include "../test/MockCardManager.h"
typedef MockCardManager CleverCardManager;
#define FREE_MANAGER
#else
#include "PKCS11CardManager.h"
typedef PKCS11CardManager CleverCardManager;
#define FREE_MANAGER if(manager != NULL) {delete manager; manager = NULL;}
#endif

#define HEX_SHA1_LENGTH (2 * BINARY_SHA1_LENGTH)
#define HEX_SHA224_LENGTH (2 * BINARY_SHA224_LENGTH)
#define HEX_SHA256_LENGTH (2 * BINARY_SHA256_LENGTH)
#define HEX_SHA512_LENGTH (2 * BINARY_SHA512_LENGTH)

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
	std::string certId;
	PinDialog *pinDialog;

	void checkReaders() {
		if (!cardManager->isReaderPresent()) {
			throw ReaderNotFoundError();
		}
	}

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

	int getRetryCount(CleverCardManager *manager) {
		int retriesLeft = manager->getPIN2RetryCount();
		if (retriesLeft == 0) {
			throw PinBlockedError();
		}
		return retriesLeft;
	}

 public:

	Signer(std::string hash, std::string certId) : hash(hash), certId(certId) {
		cardManager = NULL;
		pinDialog = NULL;
#ifndef _TEST
		cardManager = new PKCS11CardManager();
#endif
	}

	//Used for testing
	Signer(std::string hash, std::string certId, PinDialog *dialog, CleverCardManager *manager) : hash(hash), certId(certId) {
		cardManager = manager;
		pinDialog = dialog;
	}

	jsonxx::Object sign() {
		CleverCardManager *manager = NULL;
		int retriesLeft = 0;

		try {
			checkReaders();
			checkHash();
			manager = cardManager->getManagerForReader(getReaderIdByCertHash());
			pinDialog = createPinDialog(manager);
			retriesLeft = getRetryCount(manager);
			retriesLeftMessage(retriesLeft, true);
			pinDialog->setCardInfo(manager->getCardName() + ", " + manager->getPersonalCode());
		} catch (PinBlockedError &e) {
			FREE_MANAGER;
			_log("%s", e.what());
			showPINBlockedMessage();
			return error(e.code);
		} catch (CommonError &e) {
			FREE_MANAGER;
			_log("%s", e.what());
			return error(e.code);
		} catch (std::runtime_error &e) {
			FREE_MANAGER;
			_log("%s", e.what());
			return error(UNKNOWN_ERROR);
		}

		do {
			try {
				PinString pin(pinDialog->getPin().c_str());
				std::vector<unsigned char> hash = BinaryUtils::hex2bin(this->hash.c_str());
				std::vector<unsigned char> signature = manager->sign(hash, pin);

				jsonxx::Object json;
				json << "signature" << BinaryUtils::bin2hex(signature);

				FREE_MANAGER;
				pinDialog->hide();
				return json;
			} catch (AuthenticationError &ae) {
				if (ae.aborted) {
					FREE_MANAGER;
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
			} catch (UserCanceledError &e) {
				FREE_MANAGER;
				pinDialog->hide();
				return error(USER_CANCEL);
			} catch (CommonError &e) {
				FREE_MANAGER;
				_log("%s", e.what());
				pinDialog->hide();
				return error(e.code);
			} catch (std::runtime_error &e) {
				FREE_MANAGER;
				pinDialog->hide();
				_log("%s", e.what());
				return error(UNKNOWN_ERROR);
			}
		} while (retriesLeft > 0);
		FREE_MANAGER;
		showPINBlockedMessage();
		return error(USER_CANCEL);
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

	virtual void showPINBlockedMessage() {
		Gtk::MessageDialog alert(l10nLabels.get("PIN2 blocked"), false, Gtk::MESSAGE_ERROR);
		alert.set_title(l10nLabels.get("error"));
		alert.run();
	}

	virtual int getReaderIdByCertHash() {
		time_t currentTime = now();
		std::vector<unsigned int> availableTokens = cardManager->getAvailableTokens();
		for (auto &token : availableTokens) {
			CleverCardManager *manager = cardManager->getManagerForReader(token);
			if (!manager->isCardInReader()) {
				FREE_MANAGER;
				continue;
			}
			std::vector<unsigned char> cert = manager->getSignCert();
			std::string certHash = BinaryUtils::bin2hex(BinaryUtils::md5(cert));
			if (certHash == certId && currentTime <= manager->getValidTo()) {
				_log("Got readerId %i from certId ", token, certId.c_str());
				FREE_MANAGER;
				return token;
			}
			FREE_MANAGER;
		}
		throw CertNotFoundError();
	}

	virtual void checkHash() {
		int hashLength = hash.length();
		if (hashLength != HEX_SHA1_LENGTH && hashLength != HEX_SHA224_LENGTH && hashLength != HEX_SHA256_LENGTH && hashLength != HEX_SHA512_LENGTH) {
			throw InvalidHashError();
		}
	}

	virtual ~Signer() {
#ifndef _TEST
		if (pinDialog != NULL) {
			delete pinDialog;
			pinDialog = NULL;
		}
		if (cardManager != NULL) {
			delete cardManager;
			cardManager = NULL;
		}
#endif
	}
};

#endif	/* SIGNER_H */

