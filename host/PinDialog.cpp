/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include "PinDialog.h"
#include <iostream>
#include <gtk-3.0/gtk/gtk.h>
#include "Labels.h"

using namespace std;
using namespace Gtk;

PinDialog::PinDialog() {
  dialog = NULL;
  formElements = NULL;

#ifndef _TEST
  dialog = new Dialog(l10nLabels.get("signing"));
  formElements = new FormElementsContainer();
#endif
}

void PinDialog::draw() {
  dialog->set_border_width(10);
  dialog->set_position(Gtk::WIN_POS_CENTER);
  dialog->set_size_request(200, 100);
  dialog->set_resizable(false);

  Gtk::Box *box = dialog->get_content_area();
  box->add(formElements->container);
  formElements->container.set_spacing(7);

  formElements->container.pack_start(formElements->errorLabel, true, true, 0);
  formElements->container.pack_start(formElements->nameRow, false, false, 0);
  formElements->container.pack_start(formElements->pinRow, false, false, 0);

  formElements->nameRow.pack_start(formElements->nameLabel, false, false, 0);
  formElements->pinRow.pack_start(formElements->pin2Label, false, false, 0);
}

void PinDialog::hide() {
  dialog->hide();
}

string PinDialog::getPin() {
  return formElements->pin2Entry.get_text();
}

void PinDialog::setErrorMessage(string errorMessage) {
  formElements->setError(errorMessage);
}

void PinDialog::setCardInfo(string cardInfo) {
  formElements->setName(cardInfo);
}

PinDialog::~PinDialog() {
#ifndef _TEST
  if (dialog != NULL) {
    delete dialog;
    dialog = NULL;
  }
  if (formElements != NULL) {
    delete formElements;
    formElements = NULL;
  }
#endif
}