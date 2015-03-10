/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include <stdint.h>
#include <fcntl.h>
#include <io.h>

#include "jsonxx.h"
#include "InputParser.h"
#include "RequestHandler.h"
#include "Logger.h"

using namespace std;
using namespace jsonxx;

int main(int argc, char **argv) {
	//Necessary for sending correct message length to stout (in Windows)
	_setmode(_fileno(stdin), O_BINARY);
	_setmode(_fileno(stdout), O_BINARY);

	InputParser parser(cin);
	_log("Parsing input...");
	string request;
	string response;
	Object json;
	try
	{
		request = parser.readBody();
		RequestHandler handler(request);
		response = handler.handleRequest().json();
	}
	catch (const std::runtime_error &e)
	{
		json << "result" << "not_allowed" << "message" << e.what();
		response = json.json();
	}
	uint32_t responseLength = response.size() + 3;
	cout.write((char *)&responseLength, sizeof(responseLength));
	_log("Response(%i) %s ", responseLength, response.c_str());
	cout << response << endl;
	return EXIT_SUCCESS;
}
