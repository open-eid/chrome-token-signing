/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "BinaryUtils.h"
#include <stdio.h>
#include <stdexcept>

using namespace std;

vector<unsigned char> BinaryUtils::hex2bin(const string &hex) {
  if (hex.size() % 2 == 1)
      throw runtime_error("Hex is incorrect");

  vector<unsigned char> bin(hex.size() / 2, 0);
  unsigned char *c = &bin[0];
  const char *h = hex.c_str();
  while (*h) {
    int x;
    sscanf(h, "%2X", &x);
    *c = x;
    c++;
    h += 2;
  }
  return bin;
}

string BinaryUtils::bin2hex(const vector<unsigned char> &bin) {
  string hex(bin.size() * 2, 0);
  for (size_t j = 0; j < bin.size(); ++j)
    sprintf(&hex[j * 2], "%02X", (unsigned char) bin.at(j));
  return hex;
}
