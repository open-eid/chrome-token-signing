/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef INPUTPARSER_H
#define	INPUTPARSER_H

#include <istream>
#include <string>
#include "jsonxx.h"

class InputParser {
public:
    InputParser(std::istream& inputStream);
    virtual ~InputParser();
    jsonxx::Object readBody();
private:
    std::istream &inputStream;
    int readMessageLengthFromStream();
};

#endif	/* INPUTPARSER_H */

