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

#define BINARY_SHA1_LENGTH 20
#define BINARY_SHA224_LENGTH 28
#define BINARY_SHA256_LENGTH 32
#define BINARY_SHA384_LENGTH 48
#define BINARY_SHA512_LENGTH 64

class Signer {
public:
	static Signer * createSigner(const jsonxx::Object &jsonRequest);

	Signer(const std::string &_certInHex): certInHex(_certInHex) {}
	virtual ~Signer() = default;
	bool showInfo(const std::string &msg);
	virtual std::vector<unsigned char> sign(const std::vector<unsigned char> &digest) = 0;

	std::string getCertInHex() const { return certInHex; }

private:
	std::string certInHex;
};