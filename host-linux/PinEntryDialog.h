/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef PINENTRYDIALOG_H
#define	PINENTRYDIALOG_H

#include "PinDialog.h"
#include "error.h"

class PinEntryDialog : public PinDialog {
 public:
	PinEntryDialog() {
#ifndef _TEST
		formElements->setPIN2Label(l10nLabels.get("enter PIN2"));
		draw();
#endif
	}

	void draw() {
		PinDialog::draw();
		formElements->pinRow.pack_start(formElements->pin2Entry, false, false, 0);
		formElements->pinRow.set_spacing(7);

		formElements->pin2Entry.set_max_length(12);
		formElements->pin2Entry.set_visibility(false);
		formElements->pin2Entry.grab_focus();
		formElements->pin2Entry.set_activates_default(true);
		formElements->pin2Entry.signal_changed().connect(sigc::mem_fun(*this, &PinEntryDialog::enableButtonWhenPINLengthIsCorrect));

		formElements->cancelButton = dialog->add_button(l10nLabels.get("cancel"), GTK_RESPONSE_CANCEL);
		formElements->signButton = dialog->add_button(l10nLabels.get("sign"), GTK_RESPONSE_SIGN);
		formElements->signButton->set_sensitive(false);
		dialog->set_default_response(GTK_RESPONSE_SIGN);
		dialog->show_all_children();
	}

    std::string getPin() {
		int result = run();
		if (result != GTK_RESPONSE_SIGN) {
			throw UserCanceledError();
		}
		return readPin();
	}
	
	virtual int run() {
		return dialog->run();
	}
	
    virtual std::string readPin() {
		return PinDialog::getPin();
	}
	
 protected:
	void enableButtonWhenPINLengthIsCorrect() {
		formElements->signButton->set_sensitive(formElements->pin2Entry.get_text_length() >= 5);
	}
};

#endif	/* PINENTRYDIALOG_H */

