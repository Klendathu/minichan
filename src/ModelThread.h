#include <gtkmm.h>
#include <string.h>

#ifndef MINICHAN_MODELTHREAD
#define MINICHAN_MODELTHREAD

using namespace std;
class ModelThread : public Gtk::TreeModel::ColumnRecord {
	///The model for an OP
	public:

		ModelThread()
		{ add(text); add(tim); add(ext); add(no); add(fn);}

		Gtk::TreeModelColumn<string> text;
		Gtk::TreeModelColumn<string> tim;
		Gtk::TreeModelColumn<string> ext;
		Gtk::TreeModelColumn<string> no;
		Gtk::TreeModelColumn<string> fn;
};

#endif // MINICHAN_MODELTHREAD
