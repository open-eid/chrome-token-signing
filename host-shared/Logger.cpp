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
