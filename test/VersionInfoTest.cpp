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
#include "VersionInfo.h"

using namespace testing;
using namespace std;

TEST(VersionInfo, getVersion) {
  VersionInfo version;
  jsonxx::Object versionJson = version.getVersion();
  
  ASSERT_EQ(1, versionJson.size());
  ASSERT_EQ(VERSION, versionJson.get<string>("version"));
}