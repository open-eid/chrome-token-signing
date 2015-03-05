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
#include "Logger.h"
#include "Signer.h"
#include "VersionInfo.h"
#include "CertificateSelection.h"

using namespace std;
using namespace BinaryUtils;
using namespace jsonxx;

string handleJsonRequest(Object &json);

int main(int argc, char **argv) {

	InputParser parser(cin);
	_log("Parsing input...");
	string response;
	Object json;
	try
	{
		json = parser.readBody();
		response = handleJsonRequest(json);
	}
	catch (const std::runtime_error &e)
	{
		json << "returnCode" << 5 << "message" << e.what();
		response = json.json();
	}

	int responseLength = response.size();
	unsigned char *responseLengthAsBytes = intToBytesLittleEndian(responseLength);
	cout.write((char *)responseLengthAsBytes, 4);
	_log("Response(%i) %s ", responseLength, response.c_str());
	cout << response << endl;
	free(responseLengthAsBytes);
	return EXIT_SUCCESS;
}

string handleJsonRequest(Object &json) {
	jsonxx::Object responseJson;
	string response;

	if (!json.has<string>("type")) {
		responseJson << "returnCode" << 5 << "message" << "TYPE is required";
		return responseJson.json();
	}
	else {
		string type = json.get<string>("type");

		if (type == "SIGN") {
			string hashFromStdIn = json.get<String>("hash");
			string cert = json.get<String>("cert");
			_log("signing hash: %s, with certId: %s", hashFromStdIn.c_str(), cert.c_str());
			Signer signer(hashFromStdIn, cert);
			response = signer.sign().json();
		}
		else if (type == "CERT") {
			CertificateSelection cert;
			response = cert.getCert().json();
		}
		else if (type == "VERSION") {
			VersionInfo version;
			response = version.getVersion().json();
		}
		return response;
	}
}
