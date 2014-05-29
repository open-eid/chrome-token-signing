/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef LABELS_H
#define	LABELS_H

#define USER_CANCEL 1
#define READER_NOT_FOUND 5
#define UNKNOWN_ERROR 5
#define CERT_NOT_FOUND 2
#define INVALID_HASH 17
#define ONLY_HTTPS_ALLOWED 19

#include <map>
#include <string>
#include <vector>

class Labels {
 private:
	int selectedLanguage;
	std::map<std::string,std::string[3]> labels;
	std::map<int,std::string[3]> errors;
	std::map<std::string, int> languages;
	void init();

 public:
  Labels();
	void setLanguage(std::string language);
	std::string get(std::string labelKey);
	std::string getError(int errorCode);
};

extern Labels l10nLabels;

#endif	/* LABELS_H */

