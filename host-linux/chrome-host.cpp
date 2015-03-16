/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "Signer.h"
#include "CertificateSelection.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSocketNotifier>

using namespace std;
using namespace BinaryUtils;

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
    void write(QVariantMap &resp, const QString &nonce = QString()) const;

    QFile in;
    QString origin, cert;
};

void Application::parse()
{
    uint32_t messageLength = 0;
    while (in.peek((char*)&messageLength, sizeof(messageLength)) > 0) {
        QVariantMap resp;
        in.read((char*)&messageLength, sizeof(messageLength));
        if (messageLength > 1024*8)
        {
            qDebug() << "Invalid message length" << messageLength;
            resp = {{"result", "invalid_argument"}};
            write(resp);
            return exit(EXIT_FAILURE);
        }

        QJsonObject json = QJsonDocument::fromJson(in.read(messageLength)).object();
        if(json.isEmpty()) {
            resp = {{"result", "invalid_argument"}};
            write(resp, json.value("nonce").toString());
            return exit(EXIT_FAILURE);
        }
        else if(!json.contains("type") || !json.contains("nonce") || !json.contains("origin")) {
            resp = {{"result", "invalid_argument"}};
            write(resp, json.value("nonce").toString());
            return exit(EXIT_FAILURE);
        } else {
            if (json.contains("lang")) {
                Labels::l10n.setLanguage(json.value("lang").toString().toStdString());
            }

            if (origin.isEmpty()) {
                origin = json.value("origin").toString();
            } else if (origin != json.value("origin").toString()) {
                resp = {{"result", "invalid_argument"}};
                write(resp, json.value("nonce").toString());
                return exit(EXIT_FAILURE);
            }

            QString type = json.value("type").toString();
            if (type == "VERSION") {
                resp = {{"version", VERSION}};
            } else if (!json.value("origin").toString().startsWith("https:")) {
                resp = {{"result", "not_allowed"}};
                write(resp, json.value("nonce").toString());
                return exit(EXIT_FAILURE);
            }
            else if (type == "SIGN") {
                if (!json.contains("cert") || !json.contains("hash")) {
                    resp = {{"result", "invalid_argument"}};
                } else {
                    resp = Signer::sign(json.value("hash").toString(), cert);
                }
            } else if (type == "CERT") {
                resp = CertificateSelection::getCert();
                cert = resp.value("cert").toString();
            } else {
                resp = {{"result", "invalid_argument"}};
            }
        }

        write(resp, json.value("nonce").toString());
    }
}

void Application::write(QVariantMap &resp, const QString &nonce) const
{
    if (!nonce.isEmpty())
        resp["nonce"] = nonce;

    if (!resp.contains("result"))
        resp["result"] = "ok";

    QByteArray response =  QJsonDocument::fromVariant(resp).toJson();
    uint32_t responseLength = response.size();
    _log("Response(%i) %s ", responseLength, response.constData());
    QFile out;
    out.open(stdout, QFile::WriteOnly);
    out.write((const char*)&responseLength, sizeof(responseLength));
    out.write(response);
}

int main(int argc, char *argv[])
{
    return Application(argc, argv).exec();
}
