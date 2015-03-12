/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "RequestHandler.h"
#include "Logger.h"
#include "Signer.h"
#include "VersionInfo.h"
#include "CertificateSelection.h"

using namespace std;

jsonxx::Object RequestHandler::handleRequest() {
	try {
		if (jsonRequest.parse(request) && hasGloballyRequiredArguments()) {
			string type = jsonRequest.get<string>("type");
			if (type == "VERSION") {
				handleVersionRequest();
			}
			else if (type == "CERT") {
				handleCertRequest();
			}
			else if (type == "SIGN" && hasSignRequestArguments()) {
				handleSignRequest();
			}
		}
		else {
			throw InvalidArgumentException("Invalid argument");
		}
	}
	catch (InvalidArgumentException &e) {
		throw;
	}
	catch (BaseException &e) {
		handleException(e);
	}
	completeResponse();
	return jsonResponse;
}

bool RequestHandler::hasGloballyRequiredArguments() {
	return jsonRequest.has<string>("type") && jsonRequest.has<string>("nonce") && jsonRequest.has<string>("origin");
}

bool RequestHandler::hasSignRequestArguments() {
	return jsonRequest.has<string>("cert") && jsonRequest.has<string>("hash");
}

void RequestHandler::validateSecureOrigin() {
	if (!jsonRequest.has<string>("origin")) {
		throw NotAllowedException("Origin is not given");
	}
	string https("https:");
	string origin = jsonRequest.get<string>("origin");
	if (origin.compare(0, https.size(), https)) {
		throw NotAllowedException("Origin doesn't contain https");
	}
}

void RequestHandler::completeResponse() {
	if (jsonRequest.has<string>("nonce")) {
		//echo nonce
		jsonResponse << "nonce" << jsonRequest.get<string>("nonce");
	}
	// check for error
	if (!jsonResponse.has<string>("result")) {
		jsonResponse << "result" << "ok";
	}
	// add API version
	jsonResponse << "api" << API;
}

void RequestHandler::handleVersionRequest() {
	jsonResponse << "version" << VERSION;
}

void RequestHandler::handleCertRequest() {
	validateSecureOrigin();
	CertificateSelection cert;
	jsonResponse << "cert" << cert.getCert();
}

void RequestHandler::handleSignRequest() {
	validateSecureOrigin();
	string hashFromStdIn = jsonRequest.get<string>("hash");
	string cert = jsonRequest.get<string>("cert");
	_log("signing hash: %s, with certId: %s", hashFromStdIn.c_str(), cert.c_str());
	Signer signer(hashFromStdIn, cert);
	jsonResponse << "signature" << signer.sign();
}

void RequestHandler::handleException(BaseException &e) {
	jsonxx::Object exceptionalJson;
	exceptionalJson << "result" << e.getErrorCode() << "message" << e.getErrorMessage();
	jsonResponse = exceptionalJson;
}