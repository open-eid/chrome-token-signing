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

#include <string>
#include <vector>

namespace BinaryUtils {
    std::vector<unsigned char> hex2bin(const std::string &hex);
    std::string bin2hex(const std::vector<unsigned char> &bin);
};

