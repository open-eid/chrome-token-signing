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

// Very simple implementation to support Gieseke crypto token with pkcs11
// TODO: change logic if multiple pkcs11 module support becomes necessary on windows
class PKCS11ModulePath {

public:
	static std::string getModulePath() {
		return "C:\\Windows\\System32\\aetpkss1.dll";
	}
	static bool isKnownAtr(const std::string &atr) {
		return "3BFD1800008031FE4553434536302D43443134352D46CD" == atr;
	}
};
