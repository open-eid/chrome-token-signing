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
#include "jsonxx.h"
#include "RequestHandler.h"
#include "Logger.h"
#include "HostExceptions.h"

using namespace std;
using namespace jsonxx;

void handleException(BaseException &e, IOCommunicator &ioCommunicator);

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
		catch (InvalidArgumentException &e)
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

void handleException(BaseException &e, IOCommunicator &ioCommunicator) {
	_log(("Handling exception: " + e.getErrorCode()).c_str());
	Object json;
	json << "result" << e.getErrorCode() << "message" << e.getErrorMessage();
	string response = json.json();
	ioCommunicator.sendMessage(response);
}