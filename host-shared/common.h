/*
* SMARTCARDPP
* 
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
* 
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*
*/

#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>
// #include "types.h"

#ifdef WIN32
#include <tchar.h>
#endif

#ifdef __APPLE__
typedef void *LPVOID;
#define __COREFOUNDATION_CFPLUGINCOM__
#endif
/*! \mainpage Smartcardpp documentation
*
* \section intro_sec Introduction
*
* Smartcardpp reference documentation. Smartcardpp is a set of c++ classes to manage
* smartcard communications and implement basic command primitives.
*/

