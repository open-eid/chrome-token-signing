/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef VERSION_H
#define	VERSION_H

#ifndef VERSION
#define VERSION "LOCAL_BUILD"
#endif
#include "jsonxx.h"

class VersionInfo {
 public:
	jsonxx::Object getVersion() {
		jsonxx::Object json;
		json << "version" << VERSION;
		return json;
	}
};

#endif	/* VERSION_H */

