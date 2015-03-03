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
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>
#include <iconv.h>
#include <string.h>
#include <openssl/md5.h>

using namespace std;

vector<unsigned char> BinaryUtils::hex2bin(const char *hex) {
  if (strlen(hex) % 2 == 1)
      throw runtime_error("Hex is incorrect");

  vector<unsigned char> bin(strlen(hex) / 2, 0);
  unsigned char *c = &bin[0];
  const char *h = hex;
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
  for (int j = 0; j < bin.size(); ++j)
    sprintf(&hex[j * 2], "%02X", (unsigned char) bin.at(j));
  return hex;
}

unsigned char *BinaryUtils::intToBytesLittleEndian(int number) {
  unsigned char *arrayOfByte = (unsigned char *) malloc(sizeof (int));
  for (int i = 0; i < 4; i++) {
    arrayOfByte[i] = (number >> (i * 8));
  }
  return arrayOfByte;
}

void BinaryUtils::cp1250_to_utf8(char *out, char *in) {
  size_t inlen = strlen(in) + 1;
  size_t outlen = strlen(in) * 2 + 1;
  iconv_t conv = iconv_open("UTF-8", "CP1250");
  iconv(conv, &in, &inlen, &out, &outlen);
  iconv_close(conv);
}

vector<unsigned char> BinaryUtils::md5(const vector<unsigned char> &data) {
  vector<unsigned char> checksum(16, 0);
  MD5(&data[0], data.size(), &checksum[0]);
  return checksum;
}
