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
public:
	static void saveCertificate(std::string certificate);
	static bool isSelectedCertificate(std::string &certificate);
	static bool isSameOrigin(std::string &origin);
};
