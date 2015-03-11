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
		catch (BaseException &e)
		{
			string msg = "Handling exception: " + e.getErrorCode();
			_log(msg.c_str());
			Object json;
			json << "result" << e.getErrorCode() << "message" << e.getErrorMessage();
			string response = json.json();
			ioCommunicator.sendMessage(response);
			break;
		}
		catch (const std::runtime_error &e)
		{
			Object json;
			json << "result" << "invalid_argument" << "message" << e.what();
			string response = json.json();
			ioCommunicator.sendMessage(response);
			break;
		}
	}
	return EXIT_SUCCESS;
}
