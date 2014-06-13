/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#include <gtk-3.0/gtk/gtk.h>
#include "CertDialog.h"
#include "Labels.h"
#include "Logger.h"

using namespace Gtk;
using namespace std;

CertDialog::CertDialog() {
  dialog = NULL;
  formElements = NULL;

#ifndef _TEST
  dialog = new Dialog(l10nLabels.get("select certificate"));
  formElements = new CertFormElementsContainer();
  draw();
#endif
}

int CertDialog::run() {
  return dialog->run();
}

void CertDialog::hide() {
  dialog->hide();
}

void CertDialog::draw() {
  dialog->set_position(Gtk::WIN_POS_CENTER);
  dialog->set_border_width(10);
  dialog->set_resizable(false);
  //dialog->set_size_request(500, 200);

  formElements->cancelButton = dialog->add_button(l10nLabels.get("cancel"), GTK_RESPONSE_CANCEL);
  formElements->selectButton = dialog->add_button(l10nLabels.get("select"), GTK_RESPONSE_OK);
  dialog->set_default_response(GTK_RESPONSE_OK);

  refTreeModel = Gtk::ListStore::create(modelColumns);
  formElements->certificates.set_model(refTreeModel);

  Gtk::Box *box = dialog->get_content_area();
  box->add(formElements->container);

  Gtk::TreeViewColumn *certColumn = new Gtk::TreeViewColumn(l10nLabels.get("certificate"), modelColumns.name);
  certColumn->set_expand(true);
  certColumn->set_resizable(true);

  formElements->certificates.append_column(*certColumn);
  formElements->certificates.append_column(l10nLabels.get("type"), modelColumns.type);
  formElements->certificates.append_column(l10nLabels.get("valid to"), modelColumns.validTo);

  formElements->container.set_size_request(500, 200);
  formElements->container.set_spacing(7);
  formElements->infoLabel.set_text(l10nLabels.get("cert info"));
  formElements->container.pack_start(formElements->infoLabel, false, false, 0);
  formElements->container.pack_start(formElements->certificates, true, true, 0);

  dialog->show_all_children();
}

void CertDialog::addRow(int certId, string CN, string type, string validTo) {
  Gtk::TreeModel::Row row = *(refTreeModel->append());
  row[modelColumns.id] = certId;
  row[modelColumns.name] = CN;
  row[modelColumns.type] = type;
  row[modelColumns.validTo] = validTo;
}

int CertDialog::getSelectedCertIndex() {
  Gtk::TreeModel::Row row = *(formElements->certificates.get_selection()->get_selected());
  unsigned int index = row[modelColumns.id];
  _log("getSelectedCertIndex = %i", index);
  return index;
}

CertDialog::~CertDialog() {
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


