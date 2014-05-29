/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef DATEUTILS_H
#define	DATEUTILS_H

#include <time.h>
#include <string>

namespace  DateUtils {
	time_t timeFromStringWithFormat(std::string date, std::string format);
	time_t timeFromString(std::string date);
	std::string timeToString(time_t time);
	time_t now();
};

#endif	/* DATEUTILS_H */

