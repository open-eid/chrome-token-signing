/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "gmock/gmock.h"
#include "Labels.h"

TEST(Labels, defaultLanguageIsET) {
  ASSERT_STREQ("Katseid jäänud:", l10nLabels.get("tries left").c_str());
  ASSERT_STREQ("Allkirjastamine katkestati", l10nLabels.getError(USER_CANCEL).c_str());
}

TEST(Labels, setLanguageToEn) {
  l10nLabels.setLanguage("en");
  ASSERT_STREQ("Tries left:", l10nLabels.get("tries left").c_str());
  ASSERT_STREQ("Signing was cancelled", l10nLabels.getError(USER_CANCEL).c_str());
  
  l10nLabels.setLanguage("eng");
  ASSERT_STREQ("Tries left:", l10nLabels.get("tries left").c_str());
  ASSERT_STREQ("Signing was cancelled", l10nLabels.getError(USER_CANCEL).c_str());

}

TEST(Labels, setLanguageToRU) {
  l10nLabels.setLanguage("ru");
  ASSERT_STREQ("Возможных попыток:", l10nLabels.get("tries left").c_str());
  ASSERT_STREQ("Подпись была отменена", l10nLabels.getError(USER_CANCEL).c_str());
  
  l10nLabels.setLanguage("rus");
  ASSERT_STREQ("Возможных попыток:", l10nLabels.get("tries left").c_str());
  ASSERT_STREQ("Подпись была отменена", l10nLabels.getError(USER_CANCEL).c_str());  
}

TEST(Labels, setLanguageToET) {
  l10nLabels.setLanguage("et");
  ASSERT_STREQ("Katseid jäänud:", l10nLabels.get("tries left").c_str());
  ASSERT_STREQ("Allkirjastamine katkestati", l10nLabels.getError(USER_CANCEL).c_str());
  
  l10nLabels.setLanguage("est");
  ASSERT_STREQ("Katseid jäänud:", l10nLabels.get("tries left").c_str());
  ASSERT_STREQ("Allkirjastamine katkestati", l10nLabels.getError(USER_CANCEL).c_str());

}