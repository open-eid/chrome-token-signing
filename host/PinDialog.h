/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <gtkmm.h>
#include "FormElementsContainer.h"

#define GTK_RESPONSE_SIGN -100

class PinDialog {
 public:
	virtual void draw();
	PinDialog();
	virtual ~PinDialog();
	virtual void hide();
	virtual std::string getPin();
	virtual void setErrorMessage(std::string errorMessage);
	virtual void setCardInfo(std::string errorMessage);
 protected:
	Gtk::Dialog *dialog;
	FormElementsContainer *formElements;
};

#endif //PINDIALOG_H