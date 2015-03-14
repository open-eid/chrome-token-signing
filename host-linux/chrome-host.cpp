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

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QSocketNotifier>

#include <iostream>
#include <iomanip>

using namespace std;
using namespace BinaryUtils;
using namespace jsonxx;

#ifndef VERSION
#define VERSION "1.0.0"
#endif

class Application: public QApplication
{
public:
    Application(int &argc, char *argv[])
        : QApplication(argc, argv)
    {
        setQuitOnLastWindowClosed(false);
        in.open(stdin, QFile::ReadOnly);
        QSocketNotifier *n1 = new QSocketNotifier(0, QSocketNotifier::Read, this);
        connect(n1, &QSocketNotifier::activated, this, &Application::parse);
    }

private:
    void parse();
    void write(Object &resp, const string &nonce = string());

    QFile in;
    string origin, cert;
};

void Application::parse()
{
    uint32_t messageLength = 0;
    while (in.peek((char*)&messageLength, sizeof(messageLength)) > 0) {
        in.read((char*)&messageLength, sizeof(messageLength));
        //messageLength = 110;
        if (messageLength > 1024*8)
        {
            qDebug() << "Invalid message length" << messageLength;
            write(Object() << "result" << "invalid_argument");
            return exit(EXIT_FAILURE);
        }

        string message(messageLength, 0);
        in.read(&message[0], messageLength);

        Object json;
        Object resp;
        if(!json.parse(message)) {
            write(Object() << "result" << "invalid_argument");
            return exit(EXIT_FAILURE);
        }
        else if(!json.has<string>("type") || !json.has<string>("nonce") || !json.has<string>("origin")) {
            write(Object() << "result" << "invalid_argument");
            return exit(EXIT_FAILURE);
        } else {
            if (json.has<string>("lang")) {
                l10nLabels.setLanguage(json.get<string>("lang"));
            }

            if (origin.empty()) {
                origin = json.get<string>("origin");
            } else if (origin != json.get<string>("origin")) {
                write(Object() << "result" << "invalid_argument");
                return exit(EXIT_FAILURE);
            }

            string type = json.get<string>("type");
            if (type == "VERSION") {
                resp << "version" << VERSION;
            } else if (json.get<string>("origin").compare(0, 6, "https:")) {
                write(Object() << "result" << "not_allowed");
                return exit(EXIT_FAILURE);
            }
            else if (type == "SIGN") {
                if (!json.has<string>("cert") || !json.has<string>("hash")) {
                    resp << "result" << "invalid_argument";
                } else {
                    string hash = json.get<String>("hash");
                    _log("signing hash: %s", hash.c_str());
                    resp = Signer::sign(hash, cert);
                }
            } else if (type == "CERT") {
                resp = CertificateSelection().getCert();
                if (resp.has<string>("cert")) {
                    cert = resp.get<string>("cert");
                } else {
                    cert.clear();
                }
            } else {
                resp << "result" << "invalid_argument";
            }
        }

        write(resp, json.get<string>("nonce"));
    }
}

void Application::write(Object &resp, const string &nonce)
{
    if (!nonce.empty())
        resp << "nonce" << nonce;

    if (!resp.has<string>("result"))
        resp << "result" << "ok";

    string response = resp.json();
    uint32_t responseLength = response.size();
    _log("Response(%i) %s ", responseLength, response.c_str());
    QFile out;
    out.open(stdout, QFile::WriteOnly);
    out.write((const char*)&responseLength, sizeof(responseLength));
    out.write(response.c_str(), response.size());
}

int main(int argc, char *argv[])
{
    return Application(argc, argv).exec();
}
