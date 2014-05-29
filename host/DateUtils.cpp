/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "DateUtils.h"
#include <iostream>

using namespace std;

time_t DateUtils::timeFromStringWithFormat(string date, string format) {
  struct tm tm_struct = {0};
  time_t _time = time(0);

  tm_struct.tm_hour = 23;
  tm_struct.tm_min = 59;
  tm_struct.tm_sec = 59;

  if (strptime(date.c_str(), format.c_str(), &tm_struct) != NULL) {
    _time = mktime(&tm_struct);
    if (tm_struct.tm_isdst) {
      _time -= 3600;
    }
  }

  return _time;
}

time_t DateUtils::timeFromString(string date) {
  return DateUtils::timeFromStringWithFormat(date, "%d.%m.%Y");
}

string DateUtils::timeToString(time_t time) {
  tm *localTime = localtime(&time);
  int bufferSize = 11;
  char buffer[bufferSize];
  strftime(buffer, bufferSize, "%d.%m.%Y", localTime);
  return string(buffer);
}

time_t DateUtils::now() {
  return time(NULL);
}


