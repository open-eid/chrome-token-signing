/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef BINARYUTILITY_H
#define	BINARYUTILITY_H

#include <vector>

namespace BinaryUtils {
  unsigned char *hex2bin(const char *hex);
  char *bin2hex(std::vector<unsigned char> bin);
  unsigned char *intToBytesLittleEndian(int number);
	void cp1250_to_utf8(char *out, char *in);
	std::vector<unsigned char> md5(std::vector<unsigned char> data);
};

#endif	/* BINARYUTILITY_H */

