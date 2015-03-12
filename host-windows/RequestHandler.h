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
#include "HostExceptions.h"
#include "ContextMaintainer.h"

class RequestHandler {
public:
	RequestHandler(const std::string &_request) : request(_request){}
	jsonxx::Object handleRequest();
private:
	const std::string request;
	jsonxx::Object jsonRequest;
	jsonxx::Object jsonResponse;

	void handleVersionRequest();
	void handleCertRequest();
	void handleSignRequest();
	void handleException(BaseException &e);
	jsonxx::Object notAllowed();
	bool hasGloballyRequiredArguments();
	bool hasSignRequestArguments();
	void validateSecureOrigin();
	void validateContext(std::string &signingCertificate);
	void validateOrigin(std::string &origin);
	void completeResponse();
};

