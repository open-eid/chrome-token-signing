/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#pragma once

#include <gtkmm.h>

class CertDialog {
public:
	CertDialog();
    ~CertDialog();
    int run();
    void addRow(int certId, const std::string &CN, const std::string &type, const std::string &validTo);
    int getSelectedCertIndex();
private:
    class CertFormElementsContainer {
     public:
        Gtk::Button *cancelButton;
        Gtk::Button *selectButton;
        Gtk::TreeView certificates;
        Gtk::VBox container;
        Gtk::Label infoLabel;

    };

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

    Gtk::Dialog *dialog = nullptr;
    CertFormElementsContainer *formElements = nullptr;
    ModelColumns modelColumns;
	Glib::RefPtr<Gtk::ListStore> refTreeModel;
};
