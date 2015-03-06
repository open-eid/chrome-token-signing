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

#include "jsonxx.h"

class RequestHandler {
public:
	RequestHandler(const jsonxx::Object &_jsonRequest) : jsonRequest(_jsonRequest){}
	jsonxx::Object handleRequest();
private:
	const jsonxx::Object jsonRequest;
	jsonxx::Object handleVersionRequest();
	jsonxx::Object handleCertRequest();
	jsonxx::Object handleSignRequest();
	bool hasGloballyRequiredArguments();
	bool hasSignRequestArguments();
	bool hasCertRequestArguments();
	bool isSecureOrigin();
	void completeResponse(jsonxx::Object &jsonResponse);
};

