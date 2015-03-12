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

#include <string>
#include <stdexcept>

using namespace std;


class BaseException : public exception {
private:
	string message;
	string errorCode;
public:
	BaseException(string code, string msg) :message(msg), errorCode(code){
	}

	virtual const char* what() const throw()
	{
		return message.c_str();
	}
	string getErrorCode(){
		return errorCode;
	}
	string getErrorMessage() {
		return message;
	}
};

class TechnicalException : public BaseException {
public:
	TechnicalException(string message) :BaseException("technical_error", message) {}
};

class InvalidArgumentException : public BaseException {
public:
	InvalidArgumentException(string message) :BaseException("invalid_argument", message) {}
};

class InvalidHashException : public BaseException {
public:
	InvalidHashException(string message) :BaseException("invalid_argument", message) {}
};

class NotAllowedException : public BaseException {
public:
	NotAllowedException(string message) :BaseException("not_allowed", message) {}
};

class UserCancelledException : public BaseException {
public:
	UserCancelledException() :BaseException("user_cancel", "User cancelled") {}
	UserCancelledException(string msg) :BaseException("user_cancel", msg) {}
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

