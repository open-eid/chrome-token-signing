/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "Logger.h"
#include <cstdio>
#ifndef _WIN32
#include <unistd.h>
#endif

void Logger::writeLog(const char *functionName, const char *fileName, int lineNumber, const char *message, ...) {
    FILE *log = fopen("/tmp/chrome-signing.log", "a");
    if (!log) {
        return;
    }
#ifndef _WIN32
    fprintf(log, "[%i]", getpid());
#endif
    fprintf(log, "%s() [%s:%i] ", functionName, fileName, lineNumber);
    va_list args;
    va_start(args, message);
    vfprintf(log, message, args);
    va_end(args);
    fprintf(log, "\n");
    fclose(log);
}
