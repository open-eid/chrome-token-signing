/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "gmock/gmock.h"
#include "BinaryUtils.h"
#include <stdexcept>

using namespace testing;
using namespace std;

TEST(BinaryUtils, convertIntegerToByteArrayLittleEndian) {
  unsigned char *bytes = BinaryUtils::intToBytesLittleEndian(258);
  vector<unsigned char> bytesAsVector(bytes, bytes + sizeof(int));
  ASSERT_THAT(bytesAsVector, ElementsAre(2, 1, 0, 0));
  free(bytes);
}

TEST(BinaryUtils, convertHEXToBinary) {
  string hex = "00FF09CC";
  unsigned char *bytes = BinaryUtils::hex2bin(hex.c_str());
  vector<unsigned char> bytesAsVector(bytes, bytes + hex.length() / 2);
  ASSERT_THAT(bytesAsVector, ElementsAre(0, 255, 9, 204));
  free(bytes);
}

TEST(BinaryUtils, convertHEXToBinary_WithOddNumberOfHexCharactersLastCharacterIsSkipped) {
  string hex = "00FF0";
  ASSERT_THROW(BinaryUtils::hex2bin(hex.c_str()), runtime_error);
}

TEST(BinaryUtils, convertBinaryToHex) {
  unsigned char binaryArray[] = {0, 255, 9, 204};
  vector<unsigned char> binary(binaryArray, binaryArray + sizeof(binaryArray));
  char *hex = BinaryUtils::bin2hex(binary);
  ASSERT_STREQ("00FF09CC", hex);
  free(hex);
}

TEST(BinaryUtils, md5) {
  unsigned char dataArray[] = {(unsigned char)0x70, (unsigned char)0x71, (unsigned char)0x72, (unsigned char)0x73};
  std::vector<unsigned char> data(dataArray, dataArray + 4);
  ASSERT_STREQ("EAB2057CF65C5488853DD19D21D4A4B0", BinaryUtils::bin2hex(BinaryUtils::md5(data)));
  
  unsigned char dataArray1[] = {(unsigned char)0x30, (unsigned char)0x31, (unsigned char)0x32, (unsigned char)0x33};
  std::vector<unsigned char> data1(dataArray1, dataArray1 + 4);
  ASSERT_STREQ("EB62F6B9306DB575C2D596B1279627A4", BinaryUtils::bin2hex(BinaryUtils::md5(data1)));
}

