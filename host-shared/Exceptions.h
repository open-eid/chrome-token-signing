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

#include <stdexcept>

class BaseException : public std::runtime_error {
private:
	std::string errorCode;
public:
    std::string getErrorCode() const { return errorCode; }
protected:
	BaseException(const std::string &code, const std::string &msg) : runtime_error(msg), errorCode(code) {}
};

class TechnicalException : public BaseException {
public:
	TechnicalException(const std::string &message) : BaseException("technical_error", message) {}
};

class InvalidArgumentException : public BaseException {
public:
	InvalidArgumentException(const std::string &message) : BaseException("invalid_argument", message) {}
};

class InvalidHashException : public BaseException {
public:
	InvalidHashException() : BaseException("invalid_argument", "Invalid Hash") {}
};

class NotAllowedException : public BaseException {
public:
	NotAllowedException(const std::string &message) : BaseException("not_allowed", message) {}
};

class UserCancelledException : public BaseException {
public:
	UserCancelledException() : BaseException("user_cancel", "User cancelled") {}
	UserCancelledException(const std::string &message) : BaseException("user_cancel", message) {}
};

class NoCertificatesException : public BaseException {
public:
	NoCertificatesException() : BaseException("no_certificates", "Cert not found") {}
};

class NotSelectedCertificateException : public BaseException {
public:
	NotSelectedCertificateException() : BaseException("invalid_argument", "Unable to sign with certificate that has not been selected by the user") {}
};

class InconsistentOriginException : public BaseException {
public:
	InconsistentOriginException() : BaseException("invalid_argument", "Request origin can't change between requests") {}
};

class AuthenticationBadInput : public BaseException {
public:
	AuthenticationBadInput() : BaseException("pin_format_error", "Authentication Bad Input") {}
};

class AuthenticationError : public BaseException {
public:
	AuthenticationError() : BaseException("pin_invalid", "Authentication error") {}
};

class PinBlockedException : public BaseException {
public:
	PinBlockedException() : BaseException("pin_blocked", "Maximum number of PIN entry attempts has been reached") {}
};

class DriverException : public BaseException {
public:
	DriverException() : BaseException("driver_error", "Failed to load driver") {}
};

class PKCS11Exception : public BaseException {
public:
	PKCS11Exception(const std::string &msg) : BaseException("technical_error", msg) {}
};

class PKCS11TokenNotRecognized : public PKCS11Exception {
public:
	PKCS11TokenNotRecognized() : PKCS11Exception("Token not recognized.") {}
};

class PKCS11TokenNotPresent : public PKCS11Exception {
public:
	PKCS11TokenNotPresent() : PKCS11Exception("Token not present.") {}
};
