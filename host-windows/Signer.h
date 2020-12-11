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

#include <memory>
#include <string>
#include <vector>

class Signer {
public:
	virtual ~Signer() = default;

	static std::unique_ptr<Signer> createSigner(const std::vector<unsigned char> &cert);
	bool showInfo(const std::string &msg);
	virtual std::vector<unsigned char> sign(const std::vector<unsigned char> &digest) = 0;

protected:
	Signer(std::vector<unsigned char> _cert) : cert(std::move(_cert)) {}

	std::vector<unsigned char> cert;
};
