/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "jsonxx.h"
#include "Signer.h"
#include "CertificateSelection.h"

#include <gtkmm.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace BinaryUtils;
using namespace jsonxx;

#ifndef VERSION
#define VERSION "LOCAL_BUILD"
#endif

int main(int argc, char **argv)
{
    Gtk::Main kit(argc, argv);

    GIOChannel *in = g_io_channel_unix_new(STDIN_FILENO);
    guint io_watch_id = g_io_add_watch(in, G_IO_IN, [](GIOChannel *source, GIOCondition condition, gpointer data) -> gboolean {
        _log("Parsing input...");
        uint32_t messageLength = 0;
        gsize read = 0;
        g_io_channel_read(source, (char*)&messageLength, sizeof(messageLength), &read);
        if (messageLength > 1024*8)
        {
            _log("Invalid message length %s", to_string(messageLength).c_str());
            return 0;
        }

        string message(messageLength, 0);
        g_io_channel_read(source, &message[0], messageLength, &read);
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

            string type = json.get<string>("type");
            if (type == "VERSION") {
                resp << "version" << VERSION;
            } else if (json.get<string>("origin").compare(0, 6, "https:")) {
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
        _log("Response(%i) %s ", responseLength, response.c_str());
        GIOChannel *out = g_io_channel_unix_new(STDOUT_FILENO);
        g_io_channel_write(out, (const char*)&responseLength, sizeof(responseLength), &read);
        g_io_channel_write(out, response.c_str(), response.size(), &read);
        g_io_channel_unref(out);
        return true;
    }, nullptr);
    g_io_channel_unref(in);
    Glib::MainLoop::create()->run();
    return EXIT_SUCCESS;
}
