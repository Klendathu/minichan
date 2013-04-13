#include <gtkmm.h>
#include <string.h>

#ifndef MINICHAN_MODELPOST
#define MINICHAN_MODELPOST

using namespace std;
class ModelPost : public Gtk::TreeModel::ColumnRecord {
	///The model for a single post
	public:

		ModelPost()
		{ add(text); add(tim); add(ext); add(no); add(file); add(tool); add(fn);}

		Gtk::TreeModelColumn<string> text;
		Gtk::TreeModelColumn<string> tim;
		Gtk::TreeModelColumn<string> ext;
		Gtk::TreeModelColumn<string> no;
		Gtk::TreeModelColumn<int> file;
		Gtk::TreeModelColumn<string> tool;
		Gtk::TreeModelColumn<string> fn;
};

#endif // MINICHAN_MODELPOST
