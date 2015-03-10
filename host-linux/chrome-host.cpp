/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include <iostream>
#include <iomanip>
#include <gtkmm.h>
#include "jsonxx.h"
#include "Logger.h"
#include "Signer.h"
#include "CertificateSelection.h"
#include "Labels.h"

using namespace std;
using namespace BinaryUtils;
using namespace jsonxx;

#ifndef VERSION
#define VERSION "LOCAL_BUILD"
#endif

int main(int argc, char **argv) {

    Gtk::Main kit(argc, argv);

    _log("Parsing input...");
    uint32_t messageLength = 0;
    cin.read((char*)&messageLength, sizeof(messageLength));
    if (messageLength > 1024*8)
    {
        _log("Invalid message length %s", to_string(messageLength).c_str());
        return 0;
    }

    string message(messageLength, 0);
    cin.read(&message[0], messageLength);
    _log("read message(%i): %s", messageLength, message.c_str());

    Object json;
    Object resp;
    if(!json.parse(message))
        resp << "result" << "invalid_argument";
    else if(!json.has<string>("type") || !json.has<string>("nonce") || !json.has<string>("origin")) {
        resp << "result" << "invalid_argument";
    } else {
        if (json.has<string>("lang")) {
            l10nLabels.setLanguage(json.get<string>("lang"));
        }

        string origin = json.get<string>("origin");
        string type = json.get<string>("type");
        if (type == "VERSION") {
            resp << "version" << VERSION;
        } else if (origin.compare(0, 6, "https:")) {
            resp << "result" << "not_allowed";
        }
        else if (type == "SIGN") {
            if (!json.has<string>("cert") || !json.has<string>("hash")) {
                resp << "result" << "invalid_argument";
            } else {
                string hash = json.get<String>("hash");
                string cert = json.get<String>("cert");
                _log("signing hash: %s, with cert: %s", hash.c_str(), cert.c_str());
                resp = Signer(hash, cert).sign();
            }
        } else if (type == "CERT") {
            resp = CertificateSelection().getCert();
        } else {
            resp << "result" << "invalid_argument";
        }
    }

    // check for error
    if (!resp.has<string>("result"))
        resp << "result" << "ok";
    // echo nonce
    if (json.has<string>("nonce"))
        resp << "nonce" << json.get<string>("nonce");
    string response = resp.json();
    uint32_t responseLength = response.size();
    cout.write((char *) &responseLength, sizeof(responseLength));
    _log("Response(%i) %s ", responseLength, response.c_str());
    cout << response << endl;
    return EXIT_SUCCESS;
}
