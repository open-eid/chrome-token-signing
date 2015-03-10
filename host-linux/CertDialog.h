/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#ifndef CERTDIALOG_H
#define	CERTDIALOG_H

#include <gtkmm.h>
#include <CertFormElementsContainer.h>

class CertDialog {
 public:
	CertDialog();
	virtual ~CertDialog();
	virtual void draw();
	virtual int run();
	virtual void hide();
    virtual void addRow(int certId, const std::string &CN, const std::string &type, const std::string &validTo);
	virtual int getSelectedCertIndex();
 protected:
    Gtk::Dialog *dialog = nullptr;
    CertFormElementsContainer *formElements = nullptr;

	class ModelColumns : public Gtk::TreeModel::ColumnRecord {
	 public:

		ModelColumns() {
			add(id);
			add(name);
			add(type);
			add(validTo);
		}

		Gtk::TreeModelColumn<unsigned int> id;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> type;
		Gtk::TreeModelColumn<Glib::ustring> validTo;
	};

	ModelColumns modelColumns;
	Glib::RefPtr<Gtk::ListStore> refTreeModel;

};

#endif	/* CERTDIALOG_H */

