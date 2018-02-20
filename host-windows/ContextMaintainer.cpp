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

#include "ContextMaintainer.h"

std::vector<unsigned char> ContextMaintainer::selectedCertificate = std::vector<unsigned char>();
std::string ContextMaintainer::savedOrigin = std::string();

void ContextMaintainer::saveCertificate(const std::vector<unsigned char> &certificate) {
	ContextMaintainer::selectedCertificate = certificate;
}

bool ContextMaintainer::isSelectedCertificate(const std::vector<unsigned char> &certificate) {
	return ContextMaintainer::selectedCertificate == certificate;
}

void ContextMaintainer::saveOrigin(const std::string &origin) {
	ContextMaintainer::savedOrigin = origin;
}

bool ContextMaintainer::isSameOrigin(const std::string &origin) {
	if (ContextMaintainer::savedOrigin.empty()) {
		saveOrigin(origin);
		return true;
	}
	return ContextMaintainer::savedOrigin == origin;
}

