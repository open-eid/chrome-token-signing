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
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRegExpValidator>
#include <QTimeLine>
#include <QVBoxLayout>

#include <future>
#include <string>

class Signer: public QDialog {
    enum {
        UserCancel = 0,
        TechnicalError = -1,
        AuthError = -2,
    };
public:
    static QVariantMap sign(const QString &hash, const QString &cert) {
        switch(hash.length())
        {
        case BINARY_SHA1_LENGTH * 2:
        case BINARY_SHA224_LENGTH * 2:
        case BINARY_SHA256_LENGTH * 2:
        case BINARY_SHA384_LENGTH * 2:
        case BINARY_SHA512_LENGTH * 2: break;
        default:
            return {{"result", "invalid_argument"}};
        }

        std::unique_ptr<PKCS11CardManager> manager;
        std::string pkcs11ModulePath(PKCS11Path::getPkcs11ModulePath());
        try {
            for (auto &token : PKCS11CardManager::instance(pkcs11ModulePath)->getAvailableTokens()) {
                manager.reset(PKCS11CardManager::instance(pkcs11ModulePath)->getManagerForReader(token));
                if (manager->getSignCert() == fromHex(cert)) {
                    break;
                }
                manager.reset();
            }

            if(!manager)
                return {{"result", "invalid_argument"}};
        } catch (const std::runtime_error &a) {
            return {{"result", "technical_error"}};
        }

        bool isInitialCheck = true;
        for (int retriesLeft = manager->getPIN2RetryCount(); retriesLeft > 0; ) {
            Signer dialog(manager->isPinpad());
            if (retriesLeft < 3) {
                dialog.errorLabel->show();
                dialog.errorLabel->setText(QString("<font color='red'><b>%1%2 %3</b></font>")
                     .arg((!isInitialCheck ? Labels::l10n.get("incorrect PIN2") : "").c_str())
                     .arg(Labels::l10n.get("tries left").c_str())
                     .arg(retriesLeft));
            }
            isInitialCheck = false;
            dialog.nameLabel->setText((manager->getCardName() + ", " + manager->getPersonalCode()).c_str());
            std::future< std::vector<unsigned char> > signature;

            if (manager->isPinpad()) {
                signature = std::async(std::launch::async, [&](){
                    std::vector<unsigned char> result;
                    try {
                        result = manager->sign(fromHex(hash), nullptr);
                        dialog.accept();
                    } catch (const AuthenticationError &) {
                        --retriesLeft;
                        dialog.done(AuthError);
                    } catch (const AuthenticationBadInput &) {
                        dialog.done(AuthError);
                    } catch (const UserCanceledError &) {
                        dialog.done(UserCancel);
                    } catch (const std::runtime_error &) {
                        dialog.done(TechnicalError);
                    }
                    return result;
                });
            }

            switch (dialog.exec())
            {
            case UserCancel:
                return {{"result", "user_cancel"}};
            case AuthError:
                continue;
            case TechnicalError:
                return {{"result", "technical_error"}};
            default:
                if (manager->isPinpad()) {
                    return {{"signature", toHex(signature.get())}};
                }
            }

            try {
                if (!manager->isPinpad()) {
                    std::vector<unsigned char> result = manager->sign(fromHex(hash),
                        dialog.pin->text().toUtf8().constData());
                    return {{"signature", toHex(result)}};
                }
            } catch (const AuthenticationBadInput &) {
            } catch (const AuthenticationError &) {
                --retriesLeft;
            } catch (const UserCanceledError &) {
                return {{"result", "user_cancel"}};
            } catch (const std::runtime_error &) {
                return {{"result", "technical_error"}};
            }
        }
        return {{"result", "pin_blocked"}};
    }

private:
    static QByteArray toHex(const std::vector<unsigned char> &data)
    {
        return QByteArray((const char*)data.data(), data.size()).toHex();
    }

    static std::vector<unsigned char> fromHex(const QString &data)
    {
        QByteArray bin = QByteArray::fromHex(data.toLatin1());
        return std::vector<unsigned char>(bin.constData(), bin.constData() + bin.size());
    }

    Signer(bool isPinpad)
        : nameLabel(new QLabel(this))
        , pinLabel(new QLabel(this))
        , errorLabel(new QLabel(this))
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(errorLabel);
        layout->addWidget(nameLabel);
        layout->addWidget(pinLabel);

        setMinimumWidth(400);
        setWindowFlags(Qt::WindowStaysOnTopHint);
        setWindowTitle(Labels::l10n.get("signing").c_str());
        pinLabel->setText(Labels::l10n.get(isPinpad ? "enter PIN2 pinpad" : "enter PIN2").c_str());
        errorLabel->setTextFormat(Qt::RichText);
        errorLabel->hide();

        if(isPinpad) {
            setWindowFlags((windowFlags()|Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint);
            progress = new QProgressBar(this);
            progress->setRange(0, 30);
            progress->setValue(progress->maximum());
            progress->setTextVisible(false);

            statusTimer = new QTimeLine(progress->maximum() * 1000, this);
            statusTimer->setCurveShape(QTimeLine::LinearCurve);
            statusTimer->setFrameRange(progress->maximum(), progress->minimum());
            connect(statusTimer, &QTimeLine::frameChanged, progress, &QProgressBar::setValue);
            statusTimer->start();

            layout->addWidget(progress);
        } else {
            buttons = new QDialogButtonBox(this);
            connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
            cancel = buttons->addButton(Labels::l10n.get("cancel").c_str(), QDialogButtonBox::RejectRole);
            ok = buttons->addButton(Labels::l10n.get("sign").c_str(), QDialogButtonBox::AcceptRole);
            ok->setEnabled(false);

            pin = new QLineEdit(this);
            pin->setEchoMode(QLineEdit::Password);
            pin->setFocus();
            pin->setValidator(new QRegExpValidator(QRegExp("\\d{5,12}"), pin));
            pin->setMaxLength(12);
            connect(pin, &QLineEdit::textEdited, [&](const QString &text){
                ok->setEnabled(text.size() >= 5);
            });

            layout->addWidget(pin);
            layout->addWidget(buttons);
        }
        show();
    }

    QLabel *nameLabel, *pinLabel, *errorLabel;
    QDialogButtonBox *buttons = nullptr;
    QPushButton *ok = nullptr, *cancel = nullptr;
    QLineEdit *pin = nullptr;
    QProgressBar *progress = nullptr;
    QTimeLine *statusTimer = nullptr;
};
