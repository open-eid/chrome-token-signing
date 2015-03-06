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


class Signer {
public:
	Signer(const std::string &_hash, const std::string &_certInHex) : hash(_hash), certInHex(_certInHex) {}
	jsonxx::Object sign();

private:
	std::string hash;
	std::string certInHex;
};