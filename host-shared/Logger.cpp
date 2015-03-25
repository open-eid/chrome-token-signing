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

void printCurrentDateTime(FILE *log) {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    int year = 1900 + ltm->tm_year;
    int month = ltm->tm_mon;
    int day = ltm->tm_mday;
    int hour = ltm->tm_hour;
    int min = ltm->tm_min;
    int sec = ltm->tm_sec;
    //Date format yyyy-MM-dd hh:mm:ss
    fprintf(log, "%i-%i-%i %i:%i:%i ",year, month, day, hour, min, sec);
}

string getLogFilePath() {
#ifdef _WIN32
    return string(getenv("TEMP"))+"\\chrome-signing.log";
#else
    return string(getenv("HOME"))+"/tmp/chrome-signing.log";
#endif
}

void Logger::writeLog(const char *functionName, const char *fileName, int lineNumber, const char *message, ...) {
    FILE *log = fopen(getLogFilePath().c_str(), "a");
    if (!log) {
        return;
    }
    printCurrentDateTime(log);
#ifndef _WIN32
    fprintf(log, "[%i] ", getpid());
#endif
    fprintf(log, "%s() [%s:%i] ", functionName, fileName, lineNumber);
    va_list args;
    va_start(args, message);
    vfprintf(log, message, args);
    va_end(args);
    fprintf(log, "\n");
    fclose(log);
}
