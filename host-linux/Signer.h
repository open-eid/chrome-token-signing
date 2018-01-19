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
#include <QSslCertificate>
#include <QTimeLine>
#include <QVBoxLayout>

#include <future>
#include <string>

#include <openssl/x509v3.h>

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

        std::vector<unsigned char> data = fromHex(cert);
        PKCS11CardManager::Token selected;
        PKCS11Path::Params p11 = PKCS11Path::getPkcs11ModulePath();
        try {
            for (const PKCS11CardManager::Token &token : PKCS11CardManager::instance(p11.path)->tokens()) {
                if (token.cert == data) {
                    selected = token;
                    break;
                }
            }
        } catch (const std::runtime_error &a) {
            return {{"result", "technical_error"}};
        }

        if(selected.cert.empty())
            return {{"result", "invalid_argument"}};

        QSslCertificate c(QByteArray::fromRawData((const char*)data.data(), data.size()), QSsl::Der);
        ASN1_BIT_STRING *keyusage = (ASN1_BIT_STRING*)X509_get_ext_d2i((X509*)c.handle(), NID_key_usage, 0, 0);
        const int keyUsageNonRepudiation = 1;
        QString label;
        if (ASN1_BIT_STRING_get_bit(keyusage, keyUsageNonRepudiation)) {
            label = Labels::l10n.get(selected.pinpad ? "sign PIN pinpad" : "sign PIN").c_str();
            label.replace("@PIN@", p11.signPINLabel.c_str());
        }
        else {
            label = Labels::l10n.get(selected.pinpad ? "auth PIN pinpad" : "auth PIN").c_str();
            label.replace("@PIN@", p11.authPINLabel.c_str());
        }

        bool isInitialCheck = true;
        for (int retriesLeft = selected.retry; retriesLeft > 0; ) {
            Signer dialog(label, selected.minPinLen, selected.pinpad);
            if (retriesLeft < 3) {
                dialog.errorLabel->show();
                dialog.errorLabel->setText(QString("<font color='red'><b>%1%2 %3</b></font>")
                     .arg((!isInitialCheck ? Labels::l10n.get("incorrect PIN2") : "").c_str())
                     .arg(Labels::l10n.get("tries left").c_str())
                     .arg(retriesLeft));
            }
            isInitialCheck = false;
            dialog.nameLabel->setText(c.subjectInfo(QSslCertificate::CommonName).join(""));
            std::future< std::vector<unsigned char> > signature;

            if (selected.pinpad) {
                signature = std::async(std::launch::async, [&](){
                    std::vector<unsigned char> result;
                    try {
                        result = PKCS11CardManager::instance(p11.path)->sign(selected, fromHex(hash), nullptr);
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
                if (selected.pinpad) {
                    return {{"signature", toHex(signature.get())}};
                }
            }

            try {
                if (!selected.pinpad) {
                    std::vector<unsigned char> result = PKCS11CardManager::instance(p11.path)->sign(
                        selected, fromHex(hash), dialog.pin->text().toUtf8().constData());
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
        return QByteArray::fromRawData((const char*)data.data(), data.size()).toHex();
    }

    static std::vector<unsigned char> fromHex(const QString &data)
    {
        QByteArray bin = QByteArray::fromHex(data.toLatin1());
        return std::vector<unsigned char>(bin.cbegin(), bin.cend());
    }

    Signer(const QString &label, unsigned long minPinLen, bool isPinpad)
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
        pinLabel->setText(label);
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
            ok = buttons->addButton("OK", QDialogButtonBox::AcceptRole);
            ok->setEnabled(false);

            pin = new QLineEdit(this);
            pin->setEchoMode(QLineEdit::Password);
            pin->setFocus();
            pin->setValidator(new QRegExpValidator(QRegExp(QString("\\d{%1,12}").arg(minPinLen)), pin));
            pin->setMaxLength(12);
            connect(pin, &QLineEdit::textEdited, [=](const QString &text){
                ok->setEnabled(text.size() >= int(minPinLen));
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
