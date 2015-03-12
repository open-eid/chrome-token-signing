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

#include <map>
#include <string>
#include <vector>

class Labels {
 private:
	int selectedLanguage;
    std::map<std::string,std::vector<std::string> > labels;

 public:
    Labels();
	void setLanguage(const std::string &language);
    std::string get(const std::string &labelKey) const;
};

extern Labels l10nLabels;
