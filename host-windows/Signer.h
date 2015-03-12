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

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA384_LENGTH 48
#define BINARY_SHA512_LENGTH 64

class Signer {
public:
	Signer(const std::string &_hash, const std::string &_certInHex) : hash(_hash), certInHex(_certInHex) {}
	std::string sign();

private:
	std::string hash;
	std::string certInHex;
	void checkHash();
};