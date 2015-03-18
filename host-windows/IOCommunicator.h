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

using namespace std;

class IOCommunicator {
public:
	IOCommunicator();
	string readMessage() const;
	void sendMessage(const string &msg);
};