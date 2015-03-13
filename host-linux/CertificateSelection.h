/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#pragma once

#include "PKCS11CardManager.h"
#include "BinaryUtils.h"
#include "jsonxx.h"
#include "Labels.h"

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

using namespace jsonxx;

class CertificateSelection: public QDialog {
public:
    CertificateSelection()
        : message(new QLabel(this))
        , table(new QTreeWidget(this))
        , buttons(new QDialogButtonBox(this))
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(message);
        layout->addWidget(table);
        layout->addWidget(buttons);

        setWindowFlags(Qt::WindowStaysOnTopHint);
        setWindowTitle(l10nLabels.get("select certificate").c_str());
        message->setText(l10nLabels.get("cert info").c_str());

        table->setColumnCount(3);
        table->setRootIsDecorated(false);
        table->setHeaderLabels(QStringList()
            << l10nLabels.get("certificate").c_str()
            << l10nLabels.get("type").c_str()
            << l10nLabels.get("valid to").c_str());
        table->header()->setStretchLastSection(false);
        table->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->header()->setSectionResizeMode(0, QHeaderView::Stretch);

        ok = buttons->addButton(l10nLabels.get("select").c_str(), QDialogButtonBox::AcceptRole);
        cancel = buttons->addButton(l10nLabels.get("cancel").c_str(), QDialogButtonBox::RejectRole);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(table, &QTreeWidget::clicked, [&](){
            ok->setEnabled(true);
        });

        show();
    }

    Object getCert()
    {
        try {
            time_t currentTime = DateUtils::now();
            std::vector<std::string> certs;
            for (auto &token : PKCS11CardManager::instance()->getAvailableTokens()) {
                PKCS11CardManager *manager = PKCS11CardManager::instance()->getManagerForReader(token);
                time_t validTo = manager->getValidTo();
                if (currentTime <= validTo) {
                    table->insertTopLevelItem(0, new QTreeWidgetItem(table, QStringList()
                        << manager->getCN().c_str()
                        << manager->getType().c_str()
                        << DateUtils::timeToString(validTo).c_str()));
                    certs.push_back(BinaryUtils::bin2hex(manager->getSignCert()));
                }
                delete manager;
            }
            if (certs.empty())
                return Object() << "result" << "no_certificates";
            table->setCurrentIndex(table->model()->index(0, 0));
            if (exec() == 0)
                return Object() << "result" << "user_cancel";
            return Object() << "cert" << certs.at(table->currentIndex().row());
        } catch (const std::runtime_error &e) {
            qDebug() << e.what();
        }
        return Object() << "result" << "technical_error";
    }

private:
    QLabel *message;
    QTreeWidget *table;
    QDialogButtonBox *buttons;
    QPushButton *ok, *cancel;
};
