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
#include "Labels.h"

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
        try {
            time_t currentTime = DateUtils::now();
            for (auto &token : PKCS11CardManager::instance()->getAvailableTokens()) {
                manager.reset(PKCS11CardManager::instance()->getManagerForReader(token));
                if (manager->getSignCert() == BinaryUtils::hex2bin(cert.toStdString()) &&
                    currentTime <= manager->getValidTo()) {
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
                dialog.errorLabel->setText(QString("<font color='red'><b>%1%2 %3</b></font>")
                     .arg((!isInitialCheck ? l10nLabels.get("incorrect PIN2") : "").c_str())
                     .arg(l10nLabels.get("tries left").c_str())
                     .arg(retriesLeft));
            }
            isInitialCheck = false;
            dialog.nameLabel->setText((manager->getCardName() + ", " + manager->getPersonalCode()).c_str());
            std::future< std::vector<unsigned char> > signature;

            if (manager->isPinpad()) {
                signature = std::async(std::launch::async, [&](){
                    std::vector<unsigned char> result;
                    try {
                        result = manager->sign(BinaryUtils::hex2bin(hash.toStdString()), nullptr);
                        dialog.accept();
                    } catch (const AuthenticationError &) {
                        --retriesLeft;
                        dialog.done(-2);
                    } catch (const AuthenticationBadInput &) {
                        dialog.done(-2);
                    } catch (const UserCanceledError &) {
                        dialog.reject();
                    } catch (const std::runtime_error &) {
                        dialog.done(-1);
                    }
                    return result;
                });
            }

            switch (dialog.exec())
            {
            case 0:
                return {{"result", "user_cancel"}};
            case -2:
                continue;
            case -1:
                return {{"result", "technical_error"}};
            default:
                if (manager->isPinpad()) {
                    return {{"signature", BinaryUtils::bin2hex(signature.get()).c_str()}};
                }
            }

            try {
                if (!manager->isPinpad()) {
                    std::vector<unsigned char> result = manager->sign(
                        BinaryUtils::hex2bin(hash.toStdString()),
                        dialog.pin->text().toUtf8().constData());
                    return {{"signature", BinaryUtils::bin2hex(result).c_str()}};
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
    Signer(bool isPinpad)
        : nameLabel(new QLabel(this))
        , pinLabel(new QLabel(this))
        , errorLabel(new QLabel(this))
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(errorLabel);
        layout->addWidget(nameLabel);
        layout->addWidget(pinLabel);

        setMinimumWidth(350);
        setWindowFlags(Qt::WindowStaysOnTopHint);
        setWindowTitle(l10nLabels.get("signing").c_str());
        pinLabel->setText(l10nLabels.get(isPinpad ? "enter PIN2 pinpad" : "enter PIN2").c_str());
        errorLabel->setTextFormat(Qt::RichText);

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
            cancel = buttons->addButton(l10nLabels.get("cancel").c_str(), QDialogButtonBox::RejectRole);
            ok = buttons->addButton(l10nLabels.get("sign").c_str(), QDialogButtonBox::AcceptRole);
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
