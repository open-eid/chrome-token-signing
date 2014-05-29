/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "InputParser.h"
#include "Logger.h"

using namespace std;
using namespace jsonxx;

InputParser::InputParser(std::istream& inputStream):inputStream(inputStream) {
}

InputParser::~InputParser() {
}

Object InputParser::readBody() {
  int messageLength = this->readMessageLengthFromStream();
  
  char message[messageLength + 1];
  inputStream.read(message, messageLength);
  message[messageLength] = '\0';
  _log("read message(%i): %s", messageLength, message);
  Object json;
  json.parse(message);
  return json;
}

int InputParser::readMessageLengthFromStream() {
  int result = 0;
  char size[sizeof (int)];
  inputStream.read(size, sizeof (int));
  
  for (int n = sizeof (int) - 1; n >= 0; n--) {
    result = (result << 8) + (unsigned char)size[n];
  }

  return result;
}

