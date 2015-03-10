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
	if (jsonRequest.parse(request) && jsonRequest.has<string>("type")) {
		string type = jsonRequest.get<string>("type");
		if (type == "VERSION") {
			handleVersionRequest();
		}
		else if (type == "CERT" && hasCertRequestArguments()) {
			handleCertRequest();
		}
		else if (type == "SIGN" && hasSignRequestArguments()) {
			handleSignRequest();
		}
	}
	else {
		jsonResponse << "result" << "invalid_argument";
	}
	completeResponse();
	return jsonResponse;
}

bool RequestHandler::hasGloballyRequiredArguments() {
	return jsonRequest.has<string>("nonce");
}

bool RequestHandler::hasSignRequestArguments() {
	return hasGloballyRequiredArguments() && jsonRequest.has<string>("cert") && jsonRequest.has<string>("hash");
}

bool RequestHandler::hasCertRequestArguments() {
	return hasGloballyRequiredArguments();
}

bool RequestHandler::isSecureOrigin() {
	if (!jsonRequest.has<string>("origin")) {
		return false;
	}
	string https("https:");
	string origin = jsonRequest.get<string>("origin");
	return !origin.compare(0, https.size(), https);
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
	jsonResponse << "ver" << "1";
}

void RequestHandler::handleVersionRequest() {
	VersionInfo version;
	jsonResponse = version.getVersion();
}

void RequestHandler::handleCertRequest() {
	if (!isSecureOrigin()) {
		jsonResponse = notAllowed();
	}
	else {
		CertificateSelection cert;
		jsonResponse = cert.getCert();
	}
}

void RequestHandler::handleSignRequest() {
	if (!isSecureOrigin()) {
		jsonResponse = notAllowed();
	}
	else {
		string hashFromStdIn = jsonRequest.get<string>("hash");
		string cert = jsonRequest.get<string>("cert");
		_log("signing hash: %s, with certId: %s", hashFromStdIn.c_str(), cert.c_str());
		Signer signer(hashFromStdIn, cert);
		jsonResponse = signer.sign();
	}
}

jsonxx::Object RequestHandler::notAllowed() {
	jsonxx::Object json;
	json << "result" << "not_allowed";
	return json;
}