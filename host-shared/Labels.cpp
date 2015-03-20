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

#include "Labels.h"

#include <map>
#include <vector>

using namespace std;

Labels Labels::l10n = Labels();

Labels::Labels() {
    setLanguage("en");
}

void Labels::setLanguage(const string &language) {
    static std::map<std::string, int> languages = {{"et",0}, {"en",1}, {"ru",2}, {"est",0}, {"eng",1}, {"rus",2}};
    auto pos = languages.find(language);
    selectedLanguage = pos == languages.end() ? 1 : pos->second;
}

string Labels::get(const string &labelKey) const {
    static std::map<std::string,std::vector<std::string> > labels = {
        {"enter PIN2",{
            "Allkirjastamiseks sisesta PIN2:",
            "For signing enter PIN2:",
            "Для подписания введите PIN2:"
        }},
        {"enter PIN2 pinpad", {
            "Allkirjastamiseks sisesta PIN2 kaardilugeja sõrmistikult",
            "For signing enter PIN2 from PIN pad",
            "Для подписания введите PIN2 с PIN-клавиатуры"
        }},
        {"tries left",{"Katseid jäänud:", "Tries left:", "Возможных попыток:"}},
        {"incorrect PIN2",{"Vale PIN2! ", "Incorrect PIN2! ", "Неправильный PIN2! "}},
        {"signing",{"Allkirjastamine", "Signing", "Подписание"}},
        {"PIN2 blocked",{
            "PIN2 blokeeritud, ei saa allkirjastada!",
            "PIN2 blocked, cannot sign!",
            "PIN2 блокирован, невозможно подписать!"
        }},
        {"error", {"Viga", "Error", "Ошибка"}},
        {"cancel", {"Katkesta", "Cancel", "Отменить"}},
        {"sign", {"Allkirjasta", "Sign", "Подписать"}},

        {"select certificate", {"Sertifikaadi valik", "Select certificate", "Выбор сертификата"}},
        {"select", {"Vali", "Select", "Выбрать"}},
        {"certificate", {"Sertifikaat", "Certificate", "Сертификат"}},
        {"type", {"Tüüp", "Type", "Тип"}},
        {"valid to", {"Kehtiv kuni" ,"Valid to", "Действительный до"}},
        {"cert info", {
            "Sertifikaadi valikuga nõustun oma nime ja isikukoodi edastamisega teenusepakkujale.",
            "By selecting a certificate I accept that my name and personal ID code will be sent to service provider.",
            "Выбирая сертификат, я соглащаюсь с тем, что мое имя и личный код будут переданы представителю услуг."
        }}
    };
    return labels.at(labelKey)[selectedLanguage];
}
