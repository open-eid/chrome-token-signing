/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef TESTUTILS_H
#define	TESTUTILS_H
#include <string>
#include <vector>

namespace TestUtils {
    std::string getMessage(std::string);
		std::vector<unsigned int> expectedAvailableTokens(int tokenCount = 0);
};

#endif	/* TESTUTILS_H */

