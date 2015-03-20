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
