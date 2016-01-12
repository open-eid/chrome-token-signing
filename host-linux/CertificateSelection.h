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
#include <QTreeWidget>
#include <QVBoxLayout>

class CertificateSelection: public QDialog {
public:
    static QVariantMap getCert()
    {
        try {
            QList<QStringList> certs;
            std::string pkcs11ModulePath(PKCS11Path::getPkcs11ModulePath());
            for (auto &token : PKCS11CardManager::instance(pkcs11ModulePath)->getAvailableTokens()) {
                PKCS11CardManager *manager = PKCS11CardManager::instance(pkcs11ModulePath)->getManagerForReader(token);
                if (!manager -> hasSignCert()) {
                   delete manager;
                   continue;
                }
                QByteArray data((const char*)&manager->getSignCert()[0], manager->getSignCert().size());
                QSslCertificate cert(data, QSsl::Der);
                if (QDateTime::currentDateTime() < cert.expiryDate() && !isDuplicate(certs, data.toHex())) {
                    certs << (QStringList()
                        << manager->getCN().c_str()
                        << manager->getType().c_str()
                        << cert.expiryDate().toString("dd.MM.yyyy")
                        << data.toHex());
                }
                delete manager;
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
