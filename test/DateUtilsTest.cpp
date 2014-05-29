/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "gmock/gmock.h"
#include "DateUtils.h"

using namespace testing;
using namespace std;

TEST(DateUtils, timeFromString) {
  time_t seconds = DateUtils::timeFromString("03.03.2014");
  ASSERT_EQ(1393883999, seconds);
}

TEST(DateUtils, timeFromStringDaylightSaving) {
  time_t seconds = DateUtils::timeFromString("01.06.2014");
  ASSERT_EQ(1401656399, seconds);
}

TEST(DateUtils, timeToString) {
  ASSERT_EQ("12.12.2020", DateUtils::timeToString(1607810399));
}

TEST(DateUtils, now) {
  time_t now = time(NULL);
  ASSERT_THAT(DateUtils::now(), Ge(now));
}
