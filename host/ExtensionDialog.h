/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef EXTENSIONDIALOG_H
#define	EXTENSIONDIALOG_H

#ifdef _TEST
#include "../test/MockCardManager.h"
typedef MockCardManager CleverCardManager;
#define FREE_MANAGER
#else
#include "PKCS11CardManager.h"
typedef PKCS11CardManager CleverCardManager;
#define FREE_MANAGER if(manager != NULL) {delete manager; manager = NULL;}
#endif

#include "jsonxx.h"
#include "DateUtils.h"

class ExtensionDialog {
 protected:
	CleverCardManager *cardManager;

 public:
	jsonxx::Object error(int errorCode) {
		jsonxx::Object json;
		json << "returnCode" << errorCode << "message" << l10nLabels.getError(errorCode);
		return json;
	}
	
	virtual time_t now() {
		return DateUtils::now();
	}
};

#endif	/* EXTENSIONDIALOG_H */

