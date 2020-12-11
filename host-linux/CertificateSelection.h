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

class CertificateSelection: public QDialog {
public:
    static QVariantMap getCert()
    {
        try {
            QList<QStringList> certs;
            PKCS11Path::Params p11 = PKCS11Path::getPkcs11ModulePath();
            for (const PKCS11CardManager::Token &token : PKCS11CardManager(p11.path).tokens()) {
                QByteArray data = QByteArray::fromRawData((const char*)token.cert.data(), int(token.cert.size()));
                QSslCertificate cert(data, QSsl::Der);

                QString type = cert.subjectInfo(QSslCertificate::Organization).join(QString());
                bool isNonRepudiation = false;
                for(const QSslCertificateExtension &ex: cert.extensions())
                {
                    if(ex.name() == QStringLiteral("keyUsage"))
                    {
                        for(const QVariant &item: ex.value().toList())
                            if(item.toString() == QStringLiteral("Non Repudiation"))
                                isNonRepudiation = true;
                    }
                    else if(type.isEmpty() && ex.name() == QStringLiteral("certificatePolicies"))
                    {
                        QByteArray data = ex.value().toByteArray();
                        if(data.contains("1.3.6.1.4.1.51361.1.1.3") || data.contains("1.3.6.1.4.1.51361.1.2.3"))
                            type = QStringLiteral("ESTEID (DIGI-ID)");
                        else if(data.contains("1.3.6.1.4.1.51361.1.1.4") || data.contains("1.3.6.1.4.1.51361.1.2.4"))
                            type = QStringLiteral("ESTEID (DIGI-ID E-RESIDENT)");
                        else if(data.contains("1.3.6.1.4.1.51361.1") || data.contains("1.3.6.1.4.1.51455.1"))
                            type = QStringLiteral("ESTEID");
                    }
                }

                if (!isNonRepudiation) {
                    _log("certificate is not non-repu: moving on to next token...");
                    continue;
                }

                if (QDateTime::currentDateTime() < cert.expiryDate() && !isDuplicate(certs, data.toHex())) {
                    certs << QStringList({ cert.subjectInfo(QSslCertificate::CommonName).join(QString()), type,
                        cert.expiryDate().toString(QStringLiteral("dd.MM.yyyy")), data.toHex() });
                }
            }
            if (certs.empty())
                return {{"result", "no_certificates"}};
            int result = CertificateSelection(certs).exec();
            if (result == -1)
                return {{"result", "user_cancel"}};
            return {{"cert", certs.at(result)[3]}};
        } catch (const BaseException &e) {
            qDebug() << e.what();
            return {{"result", QString::fromStdString(e.getErrorCode())}};
        }
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
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel(Labels::l10n.get("cert info").c_str()));

        setWindowFlags(Qt::WindowStaysOnTopHint);
        setWindowTitle(Labels::l10n.get("select certificate").c_str());

        QTreeWidget *table = new QTreeWidget(this);
        layout->addWidget(table);
        table->setColumnCount(3);
        table->setRootIsDecorated(false);
        table->setHeaderLabels({
            Labels::l10n.get("certificate").c_str(),
            Labels::l10n.get("type").c_str(),
            Labels::l10n.get("valid to").c_str()});
        table->header()->setStretchLastSection(false);
        table->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        for(const QStringList &row: certs)
            table->insertTopLevelItem(0, new QTreeWidgetItem(table, row));
        table->setCurrentIndex(table->model()->index(0, 0));

        QDialogButtonBox *buttons = new QDialogButtonBox(this);
        layout->addWidget(buttons);
        QPushButton *ok = buttons->addButton(Labels::l10n.get("select").c_str(), QDialogButtonBox::AcceptRole);
        buttons->addButton(Labels::l10n.get("cancel").c_str(), QDialogButtonBox::RejectRole);
        connect(buttons, &QDialogButtonBox::accepted, this, [this, table] { done(table->currentIndex().row()); });
        connect(buttons, &QDialogButtonBox::rejected, this, [this] { done(-1); });
        connect(table, &QTreeWidget::clicked, [ok] { ok->setEnabled(true); });

        show();
        // Make sure window is in foreground and focus
        raise();
        activateWindow();
    }
};
