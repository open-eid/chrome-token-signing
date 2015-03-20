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
	void handleException(const BaseException &e);
	jsonxx::Object notAllowed();
	bool hasGloballyRequiredArguments();
	bool hasSignRequestArguments();
	void validateSecureOrigin();
	void validateContext(const std::string &signingCertificate);
	void validateOrigin(const std::string &origin);
	void completeResponse();
};

