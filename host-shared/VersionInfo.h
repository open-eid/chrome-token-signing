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

#define VER_STR_HELPER(x) #x
#define VER_STR(x) VER_STR_HELPER(x)

#define VERSION VER_STR(MAJOR_VER.MINOR_VER.RELEASE_VER.BUILD_VER)
#define API 1
