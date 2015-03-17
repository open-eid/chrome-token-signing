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

#include <cstdarg>

namespace Logger {
    void writeLog(const char *functionName, const char *fileName, int lineNumber, const char *message, ...);
}

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define _log(...) Logger::writeLog(__func__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define _log(...) Logger::writeLog(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#endif
