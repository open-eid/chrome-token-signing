/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef ERROR_H
#define	ERROR_H

#include "Labels.h"

#define USER_CANCEL 1
#define READER_NOT_FOUND 5
#define UNKNOWN_ERROR 5
#define CERT_NOT_FOUND 2
#define INVALID_HASH 17
#define ONLY_HTTPS_ALLOWED 19

class CommonError : public std::runtime_error {
	 public:
		int code;
		CommonError(int code, std::string message) : std::runtime_error(message), code(code) {}
};

class CertNotFoundError : public CommonError {
	public:
	 CertNotFoundError() : CommonError(CERT_NOT_FOUND, "Certificate with given hash not found"){}
};

class InvalidHashError : public CommonError {
	public:
	 InvalidHashError() : CommonError(INVALID_HASH, "Invalid hash"){}
};

class UserCanceledError : public CommonError {
	public:
	 UserCanceledError() : CommonError(USER_CANCEL, "User canceled"){}
};

class ReaderNotFoundError : public CommonError {
	public:
	 ReaderNotFoundError() : CommonError(READER_NOT_FOUND, "No readers found"){}
};

class PinBlockedError : public CommonError {
 public:
	PinBlockedError() : CommonError(USER_CANCEL, "PIN2 blocked"){}
};

class AuthenticationError : public std::runtime_error {
 public:
	bool aborted, badInput;
	AuthenticationError() : std::runtime_error(""), aborted(false), badInput(false){}
	AuthenticationError(bool aborted, bool badInput) : std::runtime_error(""), aborted(aborted), badInput(badInput){}
};

class AuthenticationErrorAborted : public AuthenticationError {
 public:
	AuthenticationErrorAborted() : AuthenticationError(true, false){}
};

class AuthenticationErrorBadInput : public AuthenticationError {
 public:
	AuthenticationErrorBadInput() : AuthenticationError(false, true){}
};

#endif	/* ERROR_H */
