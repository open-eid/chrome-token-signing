/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include <gmock/gmock.h>
#include "InputParser.h"
#include "BinaryUtils.h"
#include "TestUtils.h"

using namespace std;
using namespace jsonxx;

TEST(InputParser, readSignBody) {
  string inputString = TestUtils::getMessage("{\"type\": \"SIGN\", \"hash\": \"3021300906052B0E03021A05000414FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101\"}");
  istringstream in(inputString);

  InputParser parser(in);
  Object result = parser.readBody();
  ASSERT_STREQ("SIGN", result.get<String>("type").c_str());
  ASSERT_STREQ("3021300906052B0E03021A05000414FAFA0101FAFA0101FAFA0101FAFA0101FAFA0101", result.get<String>("hash").c_str());
}


