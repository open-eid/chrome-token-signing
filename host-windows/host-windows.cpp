/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "BinaryUtils.h"
#include "jsonxx.h"
#include "InputParser.h"
#include "RequestHandler.h"
#include "Logger.h"

using namespace std;
using namespace BinaryUtils;
using namespace jsonxx;

int main(int argc, char **argv) {

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

	int responseLength = response.length() + 2; //TODO For some reason can't get the correct size if more than 1 json key - temp. hack to fit into it
	unsigned char *responseLengthAsBytes = intToBytesLittleEndian(responseLength);
	cout.write((char *)responseLengthAsBytes, 4);
	_log("Response(%i) %s ", responseLength, response.c_str());
	cout << response << endl;
	free(responseLengthAsBytes);
	return EXIT_SUCCESS;
}
