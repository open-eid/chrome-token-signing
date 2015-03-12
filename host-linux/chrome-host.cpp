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

static void write(Object &resp, const string &nonce = string())
{
    if (!nonce.empty())
        resp << "nonce" << nonce;

    if (!resp.has<string>("result"))
        resp << "result" << "ok";

    string response = resp.json();
    uint32_t responseLength = response.size();
    _log("Response(%i) %s ", responseLength, response.c_str());
    GIOChannel *out = g_io_channel_unix_new(STDOUT_FILENO);
    gsize write;
    g_io_channel_write(out, (const char*)&responseLength, sizeof(responseLength), &write);
    g_io_channel_write(out, response.c_str(), response.size(), &write);
    g_io_channel_unref(out);
}

int main(int argc, char **argv)
{
    Gtk::Main kit(argc, argv);
    struct Data {
        Glib::RefPtr<Glib::MainLoop> main = Glib::MainLoop::create();
        string origin;
        int result = EXIT_SUCCESS;
    } data;

    GIOChannel *in = g_io_channel_unix_new(STDIN_FILENO);
    guint io_watch_id = g_io_add_watch(in, G_IO_IN, [](GIOChannel *source, GIOCondition condition, gpointer data) -> gboolean {
        Data *d = (Data*)data;

        _log("Parsing input...");
        uint32_t messageLength = 0;
        gsize read = 0;
        g_io_channel_read(source, (char*)&messageLength, sizeof(messageLength), &read);
        if (messageLength > 1024*8)
        {
            _log("Invalid message length %s", to_string(messageLength).c_str());
            write(Object() << "result" << "invalid_argument");
            d->result = EXIT_FAILURE;
            d->main->quit();
            return false;
        }

        string message(messageLength, 0);
        g_io_channel_read(source, &message[0], messageLength, &read);
        _log("read message(%i): %s", messageLength, message.c_str());

        Object json;
        Object resp;
        if(!json.parse(message)) {
            write(Object() << "result" << "invalid_argument");
            d->result = EXIT_FAILURE;
            d->main->quit();
            return false;
        }
        else if(!json.has<string>("type") || !json.has<string>("nonce") || !json.has<string>("origin")) {
            write(Object() << "result" << "invalid_argument");
            d->result = EXIT_FAILURE;
            d->main->quit();
            return false;
        } else {
            if (json.has<string>("lang")) {
                l10nLabels.setLanguage(json.get<string>("lang"));
            }

            if (d->origin.empty()) {
                d->origin = json.get<string>("origin");
            } else if (d->origin != json.get<string>("origin")) {
                write(Object() << "result" << "invalid_argument");
                d->result = EXIT_FAILURE;
                d->main->quit();
                return false;
            }

            string type = json.get<string>("type");
            if (type == "VERSION") {
                resp << "version" << VERSION;
            } else if (json.get<string>("origin").compare(0, 6, "https:")) {
                write(Object() << "result" << "not_allowed");
                d->main->quit();
                return true;
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

        write(resp, json.get<string>("nonce"));
        return true;
    }, &data);
    g_io_channel_unref(in);
    data.main->run();
    return data.result;
}
