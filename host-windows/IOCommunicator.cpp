/*
 * Chrome Token Signing Native Host
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "IOCommunicator.h"
#include "Logger.h"
#include "HostExceptions.h"
#include <stdint.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

IOCommunicator::IOCommunicator() {
	//Necessary for sending correct message length to stout (in Windows)
	_setmode(_fileno(stdin), O_BINARY);
	_setmode(_fileno(stdout), O_BINARY);
}

string IOCommunicator::readMessage() const {
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

void IOCommunicator::sendMessage(const string &message) {
	uint32_t messageLength = message.length();
	cout.write((char *)&messageLength, sizeof(messageLength));
	_log("Response(%i) %s ", messageLength, message.c_str());
	cout << message;
}
