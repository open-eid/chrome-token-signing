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

#include "ContextMaintainer.h"
#include <string>

using namespace std;

std::string ContextMaintainer::selectedCertificate;
std::string ContextMaintainer::savedOrigin;

void ContextMaintainer::saveCertificate(const string &certificate) {
	ContextMaintainer::selectedCertificate = certificate;
}

bool ContextMaintainer::isSelectedCertificate(const std::string &certificate) {
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
	else {
		return ContextMaintainer::savedOrigin == origin;
	}
}

