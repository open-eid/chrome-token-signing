/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "TestUtils.h"
#include "BinaryUtils.h"
#include <malloc.h>

using namespace std;

string TestUtils::getMessage(string json) {
  unsigned char* sizeAsArray = BinaryUtils::intToBytesLittleEndian(json.length());
  for (int i = 0; i < sizeof (int); i++) {
    json.insert(i, 1, sizeAsArray[i]);
  }
  free(sizeAsArray);
  return json;
}

vector<unsigned int> TestUtils::expectedAvailableTokens(int tokenCount) {
  vector<unsigned int> tokens;
  for (unsigned int i = 0; i < tokenCount; i++) {
    tokens.push_back(i);
  }
  return tokens;
}
 