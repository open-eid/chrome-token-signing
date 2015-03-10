/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "InputParser.h"
#include "Logger.h"
#include <stdexcept>

using namespace std;

InputParser::InputParser(std::istream& inputStream):inputStream(inputStream) {
}

InputParser::~InputParser() {
}

string InputParser::readBody() {
  int messageLength = this->readMessageLengthFromStream();
  
  if (messageLength > 1024*8)
  {
	  throw std::runtime_error("Invalid message length " + to_string(messageLength));
  }

  string message(messageLength, 0);
  inputStream.read(&message[0], messageLength);
  _log("read message(%i): %s", messageLength, message.c_str());
  return message;
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

