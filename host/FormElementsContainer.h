/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef FORMELEMENTSCONTAINER_H
#define	FORMELEMENTSCONTAINER_H

#include "Labels.h"

class FormElementsContainer {
 public:
	Gtk::Button *signButton;
	Gtk::Button *cancelButton;
	Gtk::Entry pin2Entry;

	Gtk::VBox container;
	Gtk::HBox nameRow;
	Gtk::HBox pinRow;
	Gtk::Label errorLabel;
	Gtk::Label nameLabel;
	Gtk::Label pin2Label;
	
	Gtk::ProgressBar progressBar;
	
	FormElementsContainer() {
		errorLabel.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_START);
		progressBar.set_fraction(1.0);
	}

	void setName(std::string name) {
		nameLabel.set_text(name);
	}
	
	void setError(std::string errorMessage) {
		errorLabel.set_markup("<span color='red'>" + errorMessage + "</span>");
		pin2Entry.set_text("");
	}
	
	void setPIN2Label(std::string label) {
		pin2Label.set_text(label);
	}
};

#endif	/* FORMELEMENTSCONTAINER_H */

