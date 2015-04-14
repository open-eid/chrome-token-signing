/*
 * Chrome Token Signing Native Host
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "RequestHandler.h"
#include "Logger.h"
#include "VersionInfo.h"
#include "CertificateSelection.h"
#include "ContextMaintainer.h"
#include "SignerFactory.h"

using namespace std;

jsonxx::Object RequestHandler::handleRequest() {
	try {
		if (jsonRequest.parse(request) && hasGloballyRequiredArguments()) {
			validateOrigin(jsonRequest.get<string>("origin"));
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
			else {
				throw InvalidArgumentException("Invalid argument");
			}
		}
		else {
			throw InvalidArgumentException("Invalid argument");
		}
	}
	catch (const InvalidArgumentException &e) {
		throw;
	}
	catch (const BaseException &e) {
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

void RequestHandler::validateContext(const string &signingCertificate) {
	if (!ContextMaintainer::isSelectedCertificate(signingCertificate)) {
		throw NotSelectedCertificateException();
	}
}

void RequestHandler::validateOrigin(const string &origin) {
	if (!ContextMaintainer::isSameOrigin(origin)) {
		throw InconsistentOriginException();
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
	string selectedCert = CertificateSelection::getCert();
	ContextMaintainer::saveCertificate(selectedCert);
	jsonResponse << "cert" << selectedCert;
}

void RequestHandler::handleSignRequest() {
	validateSecureOrigin();
	Signer * signer = SignerFactory::createSigner(jsonRequest);
	_log("signing hash: %s, with certId: %s", signer->getHash()->c_str(), signer->getCertInHex()->c_str());
	validateContext(*signer->getCertInHex());
	jsonResponse << "signature" << signer->sign();
}

void RequestHandler::handleException(const BaseException &e) {
	jsonxx::Object exceptionalJson;
	exceptionalJson << "result" << e.getErrorCode() << "message" << e.getErrorMessage();
	jsonResponse = exceptionalJson;
}