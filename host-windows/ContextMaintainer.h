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

class ContextMaintainer {
private:
	static std::string selectedCertificate;
	static std::string savedOrigin;
	static void saveOrigin(const std::string &origin);
public:
	static void saveCertificate(const std::string &certificate);
	static bool isSelectedCertificate(const std::string &certificate);
	static bool isSameOrigin(const std::string &origin);
};
