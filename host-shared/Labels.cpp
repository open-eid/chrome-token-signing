/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "Labels.h"

using namespace std;

Labels l10nLabels;

Labels::Labels() {
    init();
    selectedLanguage = languages.at("et");
}

void Labels::setLanguage(const string &language) {
  selectedLanguage = languages.at(language);
}

void Labels::init() {
  languages = {{"et",0}, {"en",1}, {"ru",2}, {"est",0}, {"eng",1}, {"rus",2}};
  
  labels = {
    {"enter PIN2",{"Allkirjastamiseks sisesta PIN2:", "For signing enter PIN2:", "Для подписания введите PIN2:" }},
    {"enter PIN2 pinpad", {"Allkirjastamiseks sisesta PIN2 kaardilugeja sõrmistikult", "For signing enter PIN2 from PIN pad", "Для подписания введите PIN2 с PIN-клавиатуры"}},
    {"tries left",{"Katseid jäänud:", "Tries left:", "Возможных попыток:"}},
    {"incorrect PIN2",{"Vale PIN2! ", "Incorrect PIN2! ", "Неправильный PIN2! "}},
    {"signing",{"Allkirjastamine", "Signing", "Подписание"}},
    {"PIN2 blocked",{"PIN2 blokeeritud, ei saa allkirjastada!", "PIN2 blocked, cannot sign!", "PIN2 блокирован, невозможно подписать!"}},
    {"error", {"Viga", "Error", "Ошибка"}},
    {"cancel", {"Katkesta", "Cancel", "Отменить"}},
	{"sign", {"Allkirjasta", "Sign", "Подписать"}},
    
    {"select certificate", {"Sertifikaadi valik", "Select certificate", "Выбор сертификата"}},
    {"select", {"Vali", "Select", "Выбрать"}},
    {"certificate", {"Sertifikaat", "Certificate", "Сертификат"}},
    {"type", {"Tüüp", "Type", "Тип"}},
    {"valid to", {"Kehtiv kuni" ,"Valid to", "Действительный до"}},
    {"cert info", {"Sertifikaadi valikuga nõustun oma nime ja isikukoodi edastamisega teenusepakkujale.", "By selecting a certificate I accept that my name and personal ID code will be sent to service provider.", "Выбирая сертификат, я соглащаюсь с тем, что мое имя и личный код будут переданы представителю услуг."}}
  };
        
  errors = {
    {USER_CANCEL,{"Allkirjastamine katkestati", "Signing was cancelled", "Подпись была отменена"}},
    {READER_NOT_FOUND,{"ID-kaardi lugemine ebaõnnestus", "Unable to read ID-Card", "Невозможно считать ИД-карту"}},
    {CERT_NOT_FOUND, {"Sertifikaate ei leitud", "Certificate not found", "Сертификат не найден"}},
    {INVALID_HASH, {"Vigane räsi", "Invalid hash", "Неверный хеш"}},
    {ONLY_HTTPS_ALLOWED, {"Veebis allkirjastamise käivitamine on võimalik vaid https aadressilt", "Web signing is allowed only from https:// URL", "Подпись в интернете возможна только с URL-ов, начинающихся с https://"}}
  };
}

string Labels::get(const string &labelKey) {
  return labels.at(labelKey)[selectedLanguage];
}

string Labels::getError(int errorCode) {
  return errors.at(errorCode)[selectedLanguage];
}
