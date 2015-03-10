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
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;

void Logger::writeLog(const char *functionName, const char *fileName, int lineNumber, const char *message, ...) {
  FILE *log = fopen(getLogFileName().c_str(), "a");
  if (!log) {
	  return;
  }

  string logLinePrefix = logLine(functionName, fileName, lineNumber);
  fprintf(log, "%s ", logLinePrefix.c_str());
  va_list args;
  va_start(args, message);
  vfprintf(log, message, args);
  va_end(args);
  fprintf(log, "\n");
  fclose(log);
}

string Logger::logLine(const char *functionName, const char *fileName, int lineNumber) {
  stringstream s;
#ifndef _WIN32
  s << "[" << getpid() << "] ";
#endif
  s << functionName << "() [" << fileName << ":" << lineNumber << "]";
  return s.str();
}

string Logger::getLogFileName() {
  return "/tmp/chrome-signing.log";
}
