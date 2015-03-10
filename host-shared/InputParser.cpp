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
#include <stdint.h>

using namespace std;

InputParser::InputParser(std::istream& inputStream):inputStream(inputStream) {
}

InputParser::~InputParser() {
}

string InputParser::readBody() {
	
	uint32_t messageLength = 0;
	inputStream.read((char*)&messageLength, sizeof(messageLength));
	if (messageLength > 1024 * 8)
	{
		throw std::runtime_error("Invalid message length " + to_string(messageLength));
	}
	string message(messageLength, 0);
	inputStream.read(&message[0], messageLength);
	return message;
}