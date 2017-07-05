﻿/*
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

#include "Labels.h"

#include <map>
#include <vector>

#ifdef _WIN32
#define _T(X) L##X
#define T(X) _T(X)
#else
#define T(X) X
#endif

Labels Labels::l10n = Labels();

Labels::Labels() {
    setLanguage("en");
}

void Labels::setLanguage(const std::string &language) {
    static std::map<std::string, int> languages = {
        { "et", 0 }, { "en", 1 }, { "ru", 2 }, { "lt", 3 }, { "lv", 4 },
        { "est", 0 }, { "eng", 1 }, { "rus", 2 }, { "lit", 3 }, { "lat", 4 }
    };
    auto pos = languages.find(language);
    selectedLanguage = pos == languages.end() ? 1 : pos->second;
}

Labels::lstring Labels::get(const std::string &labelKey) const {
    static const std::map<std::string,std::vector<lstring> > labels = {
    { "auth PIN", {
        T("Autentimiseks sisesta PIN:"),
        T("For authentication enter PIN:"),
        T("Для идентификации введите PIN-код:"),
        T("Norėdami patvirtinti tapatybę, įveskite PIN:"),
        T("Lai autentificētos, ievadi PIN:"),
    } },
    { "auth PIN pinpad", {
        T("Autentimiseks sisesta PIN kaardilugeja sõrmistikult"),
        T("For authentication enter PIN from PIN pad"),
        T("Для идентификации введите PIN-код при помощи клавиатуры"),
        T("Norėdami patvirtinti tapatybę, įveskite PIN, pasinaudodami klaviatūra"),
        T("Lai autentificētos, ievadi PIN no PIN ievades ierīces"),
    } },
    { "sign PIN", {
        T("Allkirjastamiseks sisesta PIN:"),
        T("For signing enter PIN:"),
        T("Для подписания введите PIN:"),
        T("Norėdami pasirašyti, įveskite PIN kodą:"),
        T("Lai parakstītu, ievadi PIN:"),
    } },
    { "sign PIN pinpad", {
        T("Allkirjastamiseks sisesta PIN kaardilugeja sõrmistikult"),
        T("For signing enter PIN from PIN pad"),
        T("Для подписания введите PIN с PIN-клавиатуры"),
        T("Norėdami pasirašyti, įveskite PIN kodą PIN klaviatūros pagalba"),
        T("Lai parakstītu, ievadi PIN paredzētajā PIN ievades ierīcē"),
    } },
    { "tries left", {
        T("Katseid jäänud:"),
        T("Tries left:"),
        T("Возможных попыток:"),
        T("Liko bandymų:"),
        T("Atlikuši mēģinājumi:"),
    } },
    { "incorrect PIN2", {
        T("Vale PIN! "),
        T("Incorrect PIN! "),
        T("Неправильный PIN! "),
        T("Neteisingas PIN! "),
        T("Nepareizs PIN! "),
    } },
    { "PIN2 blocked", {
        T("PIN2 blokeeritud, ei saa allkirjastada!"),
        T("PIN2 blocked, cannot sign!"),
        T("PIN2 блокирован, невозможно подписать!"),
        T("PIN2 užblokuotas, pasirašymas negalimas!"),
        T("PIN2 bloķēts, nav iespējams parakstīt!"),
    } },
    { "error", {
        T("Viga"),
        T("Error"),
        T("Ошибка"),
        T("Klaida"),
        T("Kļūda"),
    } },
    { "cancel", {
        T("Katkesta"),
        T("Cancel"),
        T("Отменить"),
        T("Atšaukti"),
        T("Atcelt"),
    } },
    { "sign", {
        T("Allkirjasta"),
        T("Sign"),
        T("Подписать"),
        T("Pasirašyti"),
        T("Parakstīt"),
    } },
    { "select certificate", {
        T("Sertifikaadi valik"),
        T("Select certificate"),
        T("Выбор сертификата"),
        T("Pasirinkite sertifikatą"),
        T("Izvēlēties sertifikātu"),
    } },
    { "select", {
        T("Vali"),
        T("Select"),
        T("Выбрать"),
        T("Pasirinkti"),
        T("Izvēlēties"),
    } },
    { "certificate", {
        T("Sertifikaat"),
        T("Certificate"),
        T("Сертификат"),
        T("Sertifikatas"),
        T("Sertifikāts"),
    } },
    { "type", {
        T("Tüüp"),
        T("Type"),
        T("Тип"),
        T("Tipas"),
        T("Tips"),
    } },
    { "valid to", {
        T("Kehtiv kuni"),
        T("Valid to"),
        T("Действительный до"),
        T("Galioja iki"),
        T("Derīgs līdz"),
    } },
    { "cert info", {
        T("Sertifikaadi valikuga nõustun oma nime ja isikukoodi edastamisega teenusepakkujale."),
        T("By selecting a certificate I accept that my name and personal ID code will be sent to service provider."),
        T("Выбирая сертификат, я соглащаюсь с тем, что мое имя и личный код будут переданы представителю услуг."),
        T("Pasirinkdama(s) sertifikatą, aš sutinku, kad mano vardas, pavardė ir asmens kodas būtų perduoti e. paslaugos teikėjui."),
        T("Izvēloties sertifikātu, es apstiprinu, ka mans vārds un personas kods tiks nosūtīts pakalpojuma sniedzējam."),
    } }
    };
    return labels.at(labelKey).at(selectedLanguage);
}
