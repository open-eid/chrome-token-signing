/*
 * Chrome Token Signing Native Host
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "Signer.h"
#include "CertificateSelection.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSocketNotifier>

#ifndef VERSION
#define VERSION "1.0.0.0"
#endif

class Application: public QApplication
{
public:
    Application(int &argc, char *argv[])
        : QApplication(argc, argv)
    {
        _log("Starting native host %s", VERSION);
        setWindowIcon(QIcon(":/chrome-token-signing.png"));
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
        _log("Message size: %u", messageLength);
        if (messageLength > 1024*8)
        {
            qDebug() << "Invalid message length" << messageLength;
            resp = {{"result", "invalid_argument"}};
            write(resp);
            return exit(EXIT_FAILURE);
        }

        QByteArray message = in.read(messageLength);
        _log("Message (%u): %s", messageLength, message.constData());
        QJsonObject json = QJsonDocument::fromJson(message).object();
        if(json.isEmpty()) {
            resp = {{"result", "invalid_argument"}};
            write(resp, json.value("nonce").toString());
            return exit(EXIT_FAILURE);
        }

        if(!json.contains("type") || !json.contains("nonce") || !json.contains("origin")) {
            resp = {{"result", "invalid_argument"}};
            write(resp, json.value("nonce").toString());
            return exit(EXIT_FAILURE);
        }

        if (origin.isEmpty()) {
            origin = json.value("origin").toString();
        } else if (origin != json.value("origin").toString()) {
            resp = {{"result", "invalid_argument"}};
            write(resp, json.value("nonce").toString());
            return exit(EXIT_FAILURE);
        }

        if (json.contains("lang")) {
            Labels::l10n.setLanguage(json.value("lang").toString().toStdString());
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
    _log("Response(%u) %s", responseLength, response.constData());
    QFile out;
    out.open(stdout, QFile::WriteOnly);
    out.write((const char*)&responseLength, sizeof(responseLength));
    out.write(response);
}

int main(int argc, char *argv[])
{
    return Application(argc, argv).exec();
}
