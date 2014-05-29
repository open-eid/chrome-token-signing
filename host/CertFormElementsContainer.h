/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef CERTFORMELEMENTSCONTAINER_H
#define	CERTFORMELEMENTSCONTAINER_H

class CertFormElementsContainer {
 public:
	Gtk::Button *cancelButton;
	Gtk::Button *selectButton;
	Gtk::TreeView certificates;
	Gtk::VBox container;
	Gtk::Label infoLabel;
	
};

#endif	/* CERTFORMELEMENTSCONTAINER_H */

