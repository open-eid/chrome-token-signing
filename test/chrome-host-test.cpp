/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "gmock/gmock.h"
#include "gtest/gtest.h"


int main(int argc, char *argv[]) {
//  ::testing::GTEST_FLAG(filter) = "*PKCS11CardManagerTest*";
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
