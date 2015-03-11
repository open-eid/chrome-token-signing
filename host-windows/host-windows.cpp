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
		string response;
		try
		{
			string request = ioCommunicator.readMessage();
			RequestHandler handler(request);
			response = handler.handleRequest().json();
		}
		catch (BaseException &e)
		{
			string msg = "Handling exception: " + e.getErrorCode();
			_log(msg.c_str());
			Object json;
			json << "result" << e.getErrorCode() << "message" << e.getErrorMessage();
			response = json.json();
		}
		catch (const std::runtime_error &e)
		{
			Object json;
			json << "result" << "invalid_argument" << "message" << e.what();
			response = json.json();
		}
		ioCommunicator.sendMessage(response);
	}
	return EXIT_SUCCESS;
}
