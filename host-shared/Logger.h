/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef __esteid_pkcs11__Logger__
#define __esteid_pkcs11__Logger__

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdarg>
#include <unistd.h>

namespace Logger {
  void writeLog(const char *functionName, const char *fileName, int lineNumber, std::string message, ...);
  std::string logLine(const char *functionName, const char *fileName, int lineNumber);
  std::string getLogFileName();
};


#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define _log(...) Logger::writeLog(__func__, __FILE__, __LINE__, __VA_ARGS__)
#elif (defined(__GNUC__) && __GNUC__ >= 3)
#define _log(...) Logger::writeLog(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define _log(...) Logger::writeLog(NULL, NULL, -1, ...)
#endif

#define FLOG _log("");

#endif /* defined(__esteid_pkcs11__Logger__) */
