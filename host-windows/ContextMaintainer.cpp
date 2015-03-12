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

void ContextMaintainer::saveCertificate(string &certificate) {
	ContextMaintainer::selectedCertificate = certificate;
}

bool ContextMaintainer::isSelectedCertificate(std::string &certificate) {
	return ContextMaintainer::selectedCertificate == certificate;
}
