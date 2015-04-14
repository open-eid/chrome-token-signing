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
#include "jsonxx.h"
#include "RequestHandler.h"
#include "Logger.h"
#include "HostExceptions.h"

using namespace std;
using namespace jsonxx;

void handleException(const BaseException &e, IOCommunicator &ioCommunicator);

int main(int argc, char **argv) {

	IOCommunicator ioCommunicator;

	while (true)
	{
		_log("Parsing input...");
		try
		{
			string request = ioCommunicator.readMessage();
			RequestHandler handler(request);
			string response = handler.handleRequest().json();
			ioCommunicator.sendMessage(response);
		}
		// Only catch terminating exceptions here
		catch (const InvalidArgumentException &e)
		{
			handleException(e, ioCommunicator);
			return EXIT_FAILURE;
		}
		catch (const std::runtime_error &e)
		{
			Object json;
			json << "result" << "technical_error" << "message" << string(e.what());
			string response = json.json();
			ioCommunicator.sendMessage(response);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

void handleException(const BaseException &e, IOCommunicator &ioCommunicator) {
	_log("Handling exception: %s", e.getErrorCode());
	Object json;
	json << "result" << e.getErrorCode() << "message" << e.getErrorMessage();
	string response = json.json();
	ioCommunicator.sendMessage(response);
}