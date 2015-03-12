/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "IOCommunicator.h"
#include "Logger.h"
#include "HostExceptions.h"
#include <stdint.h>
#include <fcntl.h>
#include <io.h>

IOCommunicator::IOCommunicator() {
	//Necessary for sending correct message length to stout (in Windows)
	_setmode(_fileno(stdin), O_BINARY);
	_setmode(_fileno(stdout), O_BINARY);
}

string IOCommunicator::readMessage() {
	uint32_t messageLength = 0;
	cin.read((char*)&messageLength, sizeof(messageLength));
	if (messageLength > 1024 * 8)
	{
		throw InvalidArgumentException("Invalid message length " + to_string(messageLength));
	}
	string message(messageLength, 0);
	cin.read(&message[0], messageLength);
	_log("Request(%i): %s ", messageLength, message.c_str());
	return message;
}

void IOCommunicator::sendMessage(string message) {
	uint32_t messageLength = strlen(message.c_str());
	cout.write((char *)&messageLength, sizeof(messageLength));
	_log("Response(%i) %s ", messageLength, message.c_str());
	cout << message;
}
