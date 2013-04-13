#include <gtkmm.h>
#include <string.h>

#ifndef MINICHAN_SETTINGS_DIALOG
#define MINICHAN_SETTINGS_DIALOG

using namespace std;
void nameSettingsDialog() {
	///Open the settings dialog
	
	Glib::RefPtr<Gtk::Builder> builder;
	builder = Gtk::Builder::create_from_file(DATADIR "/main.glade");
	Gtk::Window *window = 0;
	builder->get_widget("window1", window);
	
	Gtk::Dialog dialog("Settings", *window, true);
	
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	
	Gtk::Box *box = dialog.get_content_area();
	
	Gtk::Table box2(3, 3, true);
	
	Gtk::Label title_label;
	title_label.set_markup("<b>Posting Settings</b>\n");
	
	Gtk::Label name_label;
	name_label.set_text("Username ");
	Gtk::Entry name_entry;
	
	box2.attach(name_label, 0, 1, 0, 1);
	box2.attach(name_entry, 1, 3, 0, 1);
	
	Gtk::Label email_label;
	email_label.set_text("Email");
	Gtk::Entry email_entry;
	
	box2.attach(email_label, 0, 1, 1, 2);
	box2.attach(email_entry, 1, 3, 1, 2);
	
	Gtk::Label pwd_label;
	pwd_label.set_text("Password ");
	Gtk::Entry pwd_entry;
	pwd_entry.set_visibility(false);
	
	box2.attach(pwd_label, 0, 1, 2, 3);
	box2.attach(pwd_entry, 1, 3, 2, 3);
	
	Gtk::Table box3(3, 3, true);
	Gtk::Label hide_label;
	hide_label.set_text("Hide Type");
	hide_label.set_tooltip_text("What to display instead when you hide images");
	Gtk::ComboBoxText hide_type;
	hide_type.append("Special Page");
	hide_type.append("Blank Page");
	hide_type.append("Cats");
	hide_type.append("Dogs");
	hide_type.set_active(0);
	
	box3.attach(hide_label, 0, 1, 0, 1);
	box3.attach(hide_type, 1, 3, 0, 1);
	
	Gtk::Notebook nb;
	nb.append_page(box2, "Posting");
	nb.append_page(box3, "Hiding");
	box->pack_start(nb);
	
	dialog.set_attached_to(*window);
	dialog.set_transient_for(*window);
	dialog.show_all();
	int resp = dialog.run();
	switch(resp) {
		case(Gtk::RESPONSE_OK): {
			cout << "OKAY!" << endl;
			break;
		} case(Gtk::RESPONSE_CANCEL): {
			cout << "CANCEL!" << endl;
			break;
		} default: {
			cout << "U WOT M8?" << endl;
			break;
		}
	}
}

#endif // MINICHAN_SETTINGS_DIALOG
