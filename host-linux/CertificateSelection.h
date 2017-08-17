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

#pragma once

#include "PKCS11CardManager.h"
#include "Labels.h"
#include "PKCS11Path.h"

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSslCertificate>
#include <QSslCertificateExtension>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <openssl/x509v3.h>

class CertificateSelection: public QDialog {
public:
    static QVariantMap getCert(bool forSigning)
    {
        try {
            QList<QStringList> certs;
            PKCS11Path::Params p11 = PKCS11Path::getPkcs11ModulePath();
            for (const PKCS11CardManager::Token &token : PKCS11CardManager::instance(p11.path)->tokens()) {
                QByteArray data = QByteArray::fromRawData((const char*)token.cert.data(), token.cert.size());
                QSslCertificate cert(data, QSsl::Der);

                bool isCA = false;
                for(const QSslCertificateExtension &ex: cert.extensions())
                {
                    if(ex.name() == "basicConstraints")
                        isCA = ex.value().toMap().value("ca").toBool();
                }

                if (isCA)
                   continue;

                ASN1_BIT_STRING *keyusage = (ASN1_BIT_STRING*)X509_get_ext_d2i((X509*)cert.handle(), NID_key_usage, 0, 0);
                const int keyUsageNonRepudiation = 1;
                const bool isNonRepudiation = ASN1_BIT_STRING_get_bit(keyusage, keyUsageNonRepudiation);
                ASN1_BIT_STRING_free(keyusage);
                if (forSigning != isNonRepudiation) {
                    _log("certificate is non-repu: %u, requesting signing certificate %u, moving on to next token...", isNonRepudiation, forSigning);
                    continue;
                }

                if (QDateTime::currentDateTime() < cert.expiryDate() && !isDuplicate(certs, data.toHex())) {
                    certs << (QStringList()
                        << cert.subjectInfo(QSslCertificate::CommonName).join("")
                        << cert.subjectInfo(QSslCertificate::Organization).join("")
                        << cert.expiryDate().toString("dd.MM.yyyy")
                        << data.toHex());
                }
            }
            if (certs.empty())
                return {{"result", "no_certificates"}};
            CertificateSelection dialog(certs);
            if (dialog.exec() == 0)
                return {{"result", "user_cancel"}};
            return {{"cert", certs.at(dialog.table->currentIndex().row())[3]}};
        } catch (const std::runtime_error &e) {
            qDebug() << e.what();
        }
        return {{"result", "technical_error"}};
    }

private:
    static bool isDuplicate(const QList<QStringList> &certs, const QString &cert)
    {
        for(const QStringList &row : certs) {
            if(cert.compare(row[3]) == 0) return true;
        }
        return false;
    }

    CertificateSelection(const QList<QStringList> &certs)
        : message(new QLabel(this))
        , table(new QTreeWidget(this))
        , buttons(new QDialogButtonBox(this))
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(message);
        layout->addWidget(table);
        layout->addWidget(buttons);

        setWindowFlags(Qt::WindowStaysOnTopHint);
        setWindowTitle(Labels::l10n.get("select certificate").c_str());
        message->setText(Labels::l10n.get("cert info").c_str());

        table->setColumnCount(3);
        table->setRootIsDecorated(false);
        table->setHeaderLabels(QStringList()
            << Labels::l10n.get("certificate").c_str()
            << Labels::l10n.get("type").c_str()
            << Labels::l10n.get("valid to").c_str());
        table->header()->setStretchLastSection(false);
        table->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        for(const QStringList &row: certs)
            table->insertTopLevelItem(0, new QTreeWidgetItem(table, row));
        table->setCurrentIndex(table->model()->index(0, 0));

        ok = buttons->addButton(Labels::l10n.get("select").c_str(), QDialogButtonBox::AcceptRole);
        cancel = buttons->addButton(Labels::l10n.get("cancel").c_str(), QDialogButtonBox::RejectRole);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(table, &QTreeWidget::clicked, [&](){
            ok->setEnabled(true);
        });

        show();
        // Make sure window is in foreground and focus
        raise();
        activateWindow();
    }

    QLabel *message;
    QTreeWidget *table;
    QDialogButtonBox *buttons;
    QPushButton *ok = nullptr, *cancel = nullptr;
};
