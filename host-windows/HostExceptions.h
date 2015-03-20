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

#include <string>
#include <stdexcept>

using namespace std;


class BaseException : public runtime_error {
private:
	string message;
	string errorCode;
public:
	BaseException(const string &code, const string &msg) :runtime_error(msg), message(msg), errorCode(code){}

	string getErrorCode() const{
		return errorCode;
	}
	string getErrorMessage() const{
		return message;
	}
};

class TechnicalException : public BaseException {
public:
	TechnicalException(const string &message) :BaseException("technical_error", message) {}
};

class InvalidArgumentException : public BaseException {
public:
	InvalidArgumentException(const string &message) :BaseException("invalid_argument", message) {}
};

class InvalidHashException : public BaseException {
public:
	InvalidHashException() :BaseException("invalid_argument", "Invalid Hash") {}
};

class NotAllowedException : public BaseException {
public:
	NotAllowedException(const string &message) :BaseException("not_allowed", message) {}
};

class UserCancelledException : public BaseException {
public:
	UserCancelledException() :BaseException("user_cancel", "User cancelled") {}
	UserCancelledException(const string &message) :BaseException("user_cancel", message) {}
};

class NoCertificatesException : public BaseException {
public:
	NoCertificatesException() :BaseException("no_certificates", "Cert not found") {}
};

class NotSelectedCertificateException : public BaseException {
public:
	NotSelectedCertificateException() :BaseException("invalid_argument", "Unable to sign with certificate that has not been selected by the user") {}
};

class InconsistentOriginException : public BaseException {
public:
	InconsistentOriginException() :BaseException("invalid_argument", "Request origin can't change between requests") {}
};

class PinBlockedException : public BaseException {
public:
	PinBlockedException() :BaseException("pin_blocked", "Maximum number of PIN entry attempts has been reached") {}
};

