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
#include <string>
#ifndef _WIN32
#include <unistd.h>
#include <iostream>
#else
#include <time.h>
#endif

using namespace std;

void Logger::writeLog(const char *functionName, const char *fileName, int lineNumber, const char *message, ...) {
#ifdef _WIN32
    static const string path = string(getenv("TEMP")) + "\\chrome-signing.log";
#else
    static const string path = string(getenv("HOME")) + "/tmp/chrome-signing.log";
#endif

    FILE *log = fopen(path.c_str(), "r");
    if (!log) {
        return;
    }
    fclose(log);

    log = fopen(path.c_str(), "a");
    if (!log) {
        return;
    }

    time_t now = time(0);
    tm *ltm = localtime(&now);
    //Date format yyyy-MM-dd hh:mm:ss
    fprintf(log, "%i-%.2i-%.2i %.2i:%.2i:%.2i ",
            1900 + ltm->tm_year,
            ltm->tm_mon + 1,
            ltm->tm_mday,
            ltm->tm_hour,
            ltm->tm_min,
            ltm->tm_sec);

#ifndef _WIN32
    fprintf(log, "[%i] ", getpid());
#endif
    string fname(fileName);
    size_t pos = fname.find_last_of("/\\");
    if (pos != string::npos)
        fname = fname.substr(pos + 1, fname.size() - pos - 1);
    fprintf(log, "%s() [%s:%i] ", functionName, fname.c_str(), lineNumber);
    va_list args;
    va_start(args, message);
    vfprintf(log, message, args);
    va_end(args);
    fprintf(log, "\n");
    fclose(log);
}
