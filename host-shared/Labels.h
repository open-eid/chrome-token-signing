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

class Labels {
private:
	int selectedLanguage;
    Labels();

public:
    static Labels l10n;
	void setLanguage(const std::string &language);
    std::string get(const std::string &labelKey) const;
};
