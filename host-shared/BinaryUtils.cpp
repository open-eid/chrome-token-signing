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

#include "BinaryUtils.h"
#include "Exceptions.h"

#include <stdio.h>

using namespace std;

vector<unsigned char> BinaryUtils::hex2bin(const string &hex) {
  if (hex.size() % 2 == 1)
      throw InvalidArgumentException("Hex is incorrect");

  vector<unsigned char> bin(hex.size() / 2);
  unsigned char *c = bin.data();
  for (const char *h = hex.c_str(); *h; h += 2) {
    int x;
    sscanf(h, "%2X", &x);
    *c = x;
    c++;
  }
  return bin;
}

string BinaryUtils::bin2hex(const vector<unsigned char> &bin) {
    return bin2hex(bin.data(), bin.size());
}

string BinaryUtils::bin2hex(const unsigned char *bin, size_t size) {
  string hex(size * 2, 0);
  for (size_t j = 0; j < size; ++j)
    sprintf(&hex[j * 2], "%02X", (unsigned char) bin[j]);
  return hex;
}
